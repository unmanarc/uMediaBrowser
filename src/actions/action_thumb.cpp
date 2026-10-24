#include "../webclienthdlr.h"
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Helpers/encoders.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <string>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Helpers;

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
        Json::Value d;
        writeJSON(false, "File not found", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    // Generate the thumbnail in real time with ImageMagick and capture stdout.
    AppSpawn spawner;
    if (!spawner.setExec("/usr/bin/convert"))
    {
        Json::Value d;
        writeJSON(false, "ImageMagick not available", d);
        return respond(HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
    }

    spawner.addArgument(info.fullPath);
    spawner.addArgument("-auto-orient");
    spawner.addArgument("-thumbnail");
    spawner.addArgument(std::to_string(size) + "x" + std::to_string(size) + ">");
    spawner.addArgument("png:-");

    if (!spawner.spawnProcess(true, false))
    {
        Json::Value d;
        writeJSON(false, "Thumbnail generation failed", d);
        return respond(HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
    }

    std::string out;
    char buf[8192];
    ssize_t n;
    while ((n = spawner.read(1, buf, sizeof(buf))) > 0)
        out.append(buf, static_cast<size_t>(n));
    spawner.waitUntilProcessEnds();

    if (out.empty())
    {
        Json::Value d;
        writeJSON(false, "Unsupported image", d);
        return respond(HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
    }

    Json::Value data;
    data["mime"] = "image/png";
    data["b64"] = Encoders::encodeToBase64(out);

    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}