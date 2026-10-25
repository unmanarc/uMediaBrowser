#include "../webclienthdlr.h"
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Helpers/encoders.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <string>
#include <vector>

#include <fstream>
#include <unistd.h>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Helpers;

namespace
{
enum class ThumbKind
{
    IMAGE,
    VIDEO,
    RAW
};

// Classify by file extension which external tool should build the thumbnail.
ThumbKind thumbKindOf(const std::string &name)
{
    const size_t dot = name.rfind('.');
    if (dot == std::string::npos)
        return ThumbKind::IMAGE;
    std::string ext = name.substr(dot + 1);
    for (auto &c : ext)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

    if (ext == "arw" || ext == "cr2" || ext == "cr3" || ext == "nef" || ext == "orf" || ext == "dng" || ext == "pef" || ext == "raf")
        return ThumbKind::RAW;
    if (ext == "mp4" || ext == "mov" || ext == "webm" || ext == "m4v" || ext == "ogv")
        return ThumbKind::VIDEO;
    return ThumbKind::IMAGE;
}
} // namespace

// Spawn a command with `args` (first element is the executable), capture stdout.
static bool runCaptureThumb(const std::vector<std::string> &args, std::string &out)
{
    AppSpawn spawner;
    if (!spawner.setExec(args[0]))
        return false;
    for (size_t i = 1; i < args.size(); i++)
        spawner.addArgument(args[i]);
    if (!spawner.spawnProcess(true, false))
        return false;
    char buf[8192];
    ssize_t n;
    while ((n = spawner.read(1, buf, sizeof(buf))) > 0)
        out.append(buf, static_cast<size_t>(n));
    spawner.waitUntilProcessEnds();
    return true;
}

Status::Code WebClientHdlr::action_thumb()
{
    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string file = vars->getStringValue("file");

    int size = 0;
    try
    {
        size = std::stoi(vars->getStringValue("s"));
    }
    catch (...)
    {
        size = 0;
    }
    if (size <= 0)
        size = 128;
    if (size > 512)
        size = 512;

    LocalRequestedFileInfo info;
    if (!resolveExistingPath(file, info) || info.isDirectory)
    {
        logRpc(Mantids30::Program::Logs::WARNING, "R/THUMB-404: file='%s' not found", file.c_str());
        Json::Value d;
        writeJSON(false, "File not found", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    const ThumbKind kind = thumbKindOf(info.relativePath);

    std::string out;
    bool ok = false;
    std::string errReason;
    const std::string sizeArg = std::to_string(size * 2) + "x" + std::to_string(size * 2) + ">";

    if (kind == ThumbKind::VIDEO)
    {
        // Video (MP4/MOV/WEBM...): extract one frame with ffmpeg.
        std::vector<std::string> cmd = {"/usr/bin/ffmpeg",
                                        "-v",
                                        "error",
                                        "-ss",
                                        "00:00:00.5",
                                        "-i",
                                        info.fullPath,
                                        "-frames:v",
                                        "1",
                                        "-vf",
                                        "scale=" + std::to_string(size * 2) + ":-2",
                                        "-q:v",
                                        "5",
                                        "-f",
                                        "image2",
                                        "pipe:1"};
        ok = runCaptureThumb(cmd, out) && !out.empty();
        if (!ok)
            errReason = "no video frame (ffmpeg could not decode this video)";
    }
    else
    {
        // Images and RAW (ARW/CR3/NEF...):
        //  - Images: ImageMagick directly.
        //  - RAW: staged fallback: ImageMagick (decodes DNG; some builds read
        //    camera RAW previews) -> exiftool -b -PreviewImage (extracts the
        //    embedded JPEG preview cameras store inside ARW/CR3/NEF/CR2) ->
        //    dcraw (if installed) as a last resort.

        std::vector<std::string> cmd = {"/usr/bin/convert", info.fullPath, "-auto-orient", "-thumbnail", sizeArg, "-quality", "50", "jpg:-"};
        ok = runCaptureThumb(cmd, out) && !out.empty();

        if (kind == ThumbKind::RAW && !ok)
        {
            const std::string pidSuffix = std::to_string(::getpid());
            const std::string tmpPrev = "/tmp/uws_thumb_" + pidSuffix + "_prev.jpg";

            // Stage 2: extract the embedded JPEG preview with exiftool.
            std::string preview;
            if (runCaptureThumb({"/usr/bin/exiftool", "-b", "-PreviewImage", info.fullPath}, preview) && !preview.empty())
            {
                {
                    std::ofstream tf(tmpPrev, std::ios::binary);
                    tf.write(preview.data(), static_cast<std::streamsize>(preview.size()));
                }
                std::vector<std::string> t7 = {"/usr/bin/magick", tmpPrev, "-auto-orient", "-thumbnail", sizeArg, "-quality", "50", "jpg:-"};
                std::vector<std::string> t6 = {"/usr/bin/convert", tmpPrev, "-auto-orient", "-thumbnail", sizeArg, "-quality", "50", "jpg:-"};
                out.clear();
                if (runCaptureThumb(t7, out) && !out.empty())
                    ok = true;
                else
                {
                    out.clear();
                    ok = runCaptureThumb(t6, out) && !out.empty();
                }
            }
            ::unlink(tmpPrev.c_str());

            // Stage 3: dcraw (optional, e.g. dcraw-ng).
            if (!ok)
            {
                const std::string tmpTiff = "/tmp/uws_thumb_" + pidSuffix + "_raw.tif";
                const std::vector<std::string> raw = {"/usr/bin/dcraw", "-q", "2", "-T", tmpTiff, info.fullPath};
                if (runCaptureThumb(raw, out) && ::access(tmpTiff.c_str(), F_OK) == 0)
                {
                    std::vector<std::string> t7 = {"/usr/bin/magick", tmpTiff, "-auto-orient", "-thumbnail", sizeArg, "-quality", "50", "jpg:-"};
                    std::vector<std::string> t6 = {"/usr/bin/convert", tmpTiff, "-auto-orient", "-thumbnail", sizeArg, "-quality", "50", "jpg:-"};
                    out.clear();
                    if (runCaptureThumb(t7, out) && !out.empty())
                        ok = true;
                    else
                    {
                        out.clear();
                        ok = runCaptureThumb(t6, out) && !out.empty();
                    }
                }
                ::unlink(tmpTiff.c_str());
            }
        }

        if (!ok)
            errReason = (kind == ThumbKind::RAW) ? "no RAW thumbnail (ImageMagick/exiftool/dcraw could not decode this RAW file)" : "unsupported image";
    }

    if (!ok || out.empty())
    {
        logRpc(Mantids30::Program::Logs::WARNING, "R/THUMB-FAIL: file='%s' (%s)", info.relativePath.c_str(), errReason.c_str());
        Json::Value d;
        writeJSON(false, errReason, d);
        return respond(HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
    }

    Json::Value data;
    data["mime"] = "image/jpeg";
    data["b64"] = Encoders::encodeToBase64(out);

    const char *kindName = (kind == ThumbKind::RAW) ? "raw" : ((kind == ThumbKind::VIDEO) ? "video" : "image");
    logRpc(Mantids30::Program::Logs::INFO, "R/THUMB: file='%s' s=%d kind=%s -> %zu bytes jpeg",
           info.relativePath.c_str(),
           size,
           kindName,
           out.size());
    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}