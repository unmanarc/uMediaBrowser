#include "../webclienthdlr.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <Mantids30/Helpers/appexec.h>

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Helpers;

namespace
{
// Run a command capturing its stdout into `out`. Returns true if the process
// could be spawned and its output was fully read.
bool runCapture(const std::string &exe, const std::vector<std::string> &args, std::string &out)
{
    AppSpawn spawner;
    if (!spawner.setExec(exe))
        return false;
    for (const std::string &a : args)
        spawner.addArgument(a);
    if (!spawner.spawnProcess(true, false))
        return false;
    char buf[8192];
    ssize_t n;
    while ((n = spawner.read(1, buf, sizeof(buf))) > 0)
        out.append(buf, static_cast<size_t>(n));
    spawner.waitUntilProcessEnds();
    return true;
}
} // namespace

Status::Code WebClientHdlr::action_detail()
{
    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    const std::string file = vars->getStringValue("file");

    LocalRequestedFileInfo info;
    if (!resolveExistingPath(file, info))
    {
        Json::Value d;
        writeJSON(false, "File not found", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    Json::Value data;
    data["type"] = info.isDirectory ? "dir" : "file";

    if (info.isDirectory)
    {
        writeJSON(true, "", data);
        return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
    }

    // Image dimensions + format via ImageMagick identify.
    std::string idOut;
    if (runCapture("/usr/bin/identify", {"-format", "%m %w %h", info.fullPath}, idOut))
    {
        std::istringstream iss(idOut);
        std::string fmt, w, h;
        iss >> fmt >> w >> h;
        if (!fmt.empty())
        {
            if (fmt == "JPEG") data["mime"] = "image/jpeg";
            else if (fmt == "PNG") data["mime"] = "image/png";
            else if (fmt == "GIF") data["mime"] = "image/gif";
            else if (fmt == "WEBP") data["mime"] = "image/webp";
            else if (fmt == "BMP") data["mime"] = "image/bmp";
            else if (fmt == "TIFF") data["mime"] = "image/tiff";
            else data["mime"] = "image/" + fmt;
        }
        if (!w.empty())
            data["width"] = std::atoi(w.c_str());
        if (!h.empty())
            data["height"] = std::atoi(h.c_str());
    }

    // Relevant EXIF tags via exiftool (optional; empty if unavailable).
    std::string exOut;
    if (runCapture("/usr/bin/exiftool", {"-json", "-s", info.fullPath}, exOut))
    {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(exOut, root) && root.isArray() && !root.empty() && root[0].isObject())
        {
            const Json::Value &obj = root[0];
            Json::Value exif(Json::objectValue);
            static const char *keys[] = {
                "Make", "Model", "LensModel", "DateTimeOriginal", "CreateDate",
                "ExposureTime", "FNumber", "ISO", "ISOSpeedRatings", "FocalLength",
                "Software", "GPSLatitude", "GPSLongitude",
                "GPSLatitudeRef", "GPSLongitudeRef"
            };
            for (const char *k : keys)
                if (obj.isMember(k))
                    exif[k] = obj[k];
            data["exif"] = exif;
        }
    }

    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}