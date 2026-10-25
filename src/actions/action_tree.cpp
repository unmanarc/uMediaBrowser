#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;

Status::Code WebClientHdlr::action_tree()
{
    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string path = vars->getStringValue("path");

    LocalRequestedFileInfo dirInfo;
    if (!resolveExistingPath(path, dirInfo) || !dirInfo.isDirectory)
    {
        logRpc(Mantids30::Program::Logs::WARNING, "R/TREE-404: path='%s' not found or not a directory", path.c_str());
        Json::Value d;
        writeJSON(false, "Directory not found", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    const std::string rel = dirInfo.relativePath.empty() ? "/" : dirInfo.relativePath;

    Json::Value nodes(Json::arrayValue);

    std::error_code ec;
    for (auto it = fs::directory_iterator(dirInfo.fullPath, ec); !ec && it != fs::directory_iterator(); it.increment(ec))
    {
        const fs::directory_entry &entry = *it;
        const std::string name = entry.path().filename().string();

        std::error_code sec;
        if (!fs::is_directory(entry.path(), sec))
            continue;

        // Determine whether this subdirectory has any subdirectory of its own.
        bool hasChildren = false;
        std::error_code cec;
        for (auto cit = fs::directory_iterator(entry.path(), cec); !cec && cit != fs::directory_iterator(); cit.increment(cec))
        {
            std::error_code isec;
            if (fs::is_directory(cit->path(), isec) && !isec)
            {
                hasChildren = true;
                break;
            }
        }

        std::string childRel = rel;
        if (childRel.back() != '/')
            childRel += '/';
        childRel += name;

        Json::Value node;
        node["name"] = name;
        node["path"] = childRel;
        node["has_children"] = hasChildren;
        nodes.append(node);
    }

    Json::Value data;
    data["path"] = rel;
    data["nodes"] = nodes;

    logRpc(Mantids30::Program::Logs::INFO, "R/TREE: path='%s' -> %u dir nodes", rel.c_str(), (unsigned) nodes.size());
    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}