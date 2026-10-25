#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;

Status::Code WebClientHdlr::action_delete()
{
    if (!webClientParameters.overwrite)
    {
        Json::Value d;
        writeJSON(false, "Delete is disabled", d);
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
    }

    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string filesParam = vars->getStringValue("files");

    std::vector<std::string> names = ActionCommon::splitComma(filesParam);
    if (names.empty())
    {
        logRpc(Mantids30::Program::Logs::WARNING, "R/DEL-400: no paths selected");
        Json::Value d;
        writeJSON(false, "No paths selected", d);
        return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
    }

    Json::Value deleted(Json::arrayValue);
    std::string error;

    for (const std::string &f : names)
    {
        LocalRequestedFileInfo info;
        if (!resolveExistingPath(f, info))
        {
            logRpc(Mantids30::Program::Logs::WARNING, "R/DEL-404: path='%s' not found", f.c_str());
            error = "Invalid path: " + f;
            break;
        }

        const std::string rel = info.relativePath.empty() ? "/" : info.relativePath;
        if (rel == "/" || rel.empty())
        {
            logRpc(Mantids30::Program::Logs::WARNING, "R/DEL-403: refused to delete document root");
            error = "Document root cannot be deleted";
            break;
        }

        if (ActionCommon::isReservedDotFile(ActionCommon::baseName(rel)))
        {
            logRpc(Mantids30::Program::Logs::WARNING, "R/DEL-403: refused to delete reserved file '%s'", f.c_str());
            error = "Reserved control file cannot be deleted: " + f;
            break;
        }

        std::error_code ec;
        fs::remove_all(info.fullPath, ec);
        if (ec)
        {
            logRpc(Mantids30::Program::Logs::ERROR, "R/DEL-FAIL: '%s' (%s)", rel.c_str(), ec.message().c_str());
            error = "Could not delete: " + f;
            break;
        }

        deleted.append(rel);
    }

    Json::Value data;
    data["deleted"] = deleted;

    if (error.empty())
        logRpc(Mantids30::Program::Logs::INFO, "R/DEL: deleted %u path(s)", (unsigned) deleted.size());
    else
        logRpc(Mantids30::Program::Logs::WARNING, "R/DEL-ERR: %s (deleted %u before failure)", error.c_str(), (unsigned) deleted.size());

    writeJSON(error.empty(), error, data);
    return respond(error.empty() ? HTTP::Status::Code::S_200_OK : HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
}