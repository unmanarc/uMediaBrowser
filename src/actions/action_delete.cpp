#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <vector>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;

// Recursively remove a file or directory (relative to the already-validated path).
static bool removeRecursive(const std::string &path)
{
    struct stat st;
    if (lstat(path.c_str(), &st) != 0)
        return false;

    if (!S_ISDIR(st.st_mode))
        return unlink(path.c_str()) == 0;

    DIR *d = opendir(path.c_str());
    if (!d)
        return false;

    struct dirent *e;
    bool ok = true;
    while ((e = readdir(d)) != nullptr)
    {
        std::string name = e->d_name;
        if (name == "." || name == "..")
            continue;
        if (!removeRecursive(path + "/" + name))
            ok = false;
    }
    closedir(d);

    if (rmdir(path.c_str()) != 0)
        ok = false;

    return ok;
}

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
            error = "Invalid path: " + f;
            break;
        }

        const std::string rel = info.relativePath.empty()? "/" : info.relativePath;
        if (rel == "/" || rel.empty())
        {
            error = "Document root cannot be deleted";
            break;
        }

        if (ActionCommon::isReservedDotFile(ActionCommon::baseName(rel)))
        {
            error = "Reserved control file cannot be deleted: " + f;
            break;
        }

        if (!removeRecursive(info.fullPath))
        {
            error = "Could not delete: " + f;
            break;
        }

        deleted.append(rel);
    }

    Json::Value data;
    data["deleted"] = deleted;
    writeJSON(error.empty(), error, data);
    return respond(error.empty()? HTTP::Status::Code::S_200_OK : HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
}