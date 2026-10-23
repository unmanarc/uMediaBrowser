#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;

Status::Code WebClientHdlr::action_mkdir()
{
    if (!webClientParameters.uploads)
    {
        Json::Value d;
        writeJSON(false, "Folder creation is disabled", d);
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
    }

    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string path = vars->getStringValue("path");
    std::string name = vars->getStringValue("name");

    if (name.empty() || ActionCommon::isReservedDotFile(name))
    {
        Json::Value d;
        writeJSON(false, "Invalid folder name", d);
        return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
    }

    LocalRequestedFileInfo info;
    if (!resolveNewChildPath(path, name, info))
    {
        Json::Value d;
        writeJSON(false, "Invalid parent path", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    std::error_code ec;
    if (!fs::create_directory(info.fullPath, ec) || ec)
    {
        Json::Value d;
        writeJSON(false, "Could not create folder", d);
        return respond(HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
    }

    Json::Value data;
    data["path"] = info.relativePath;
    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}