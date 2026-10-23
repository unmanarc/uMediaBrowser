#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <dirent.h>
#include <sys/stat.h>

#include <string>

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
        Json::Value d;
        writeJSON(false, "Directory not found", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    const std::string rel = dirInfo.relativePath.empty()? "/" : dirInfo.relativePath;

    Json::Value nodes(Json::arrayValue);

    DIR *dirp = opendir(dirInfo.fullPath.c_str());
    if (dirp)
    {
        struct dirent *dp;
        while ((dp = readdir(dirp)) != nullptr)
        {
            std::string name = dp->d_name;
            if (name == "." || name == "..")
                continue;

            std::string full = dirInfo.fullPath + "/" + name;
            struct stat st;
            if (stat(full.c_str(), &st) != 0)
                continue;
            if (!S_ISDIR(st.st_mode))
                continue;

            // Determine whether this subdirectory has any subdirectory of its own.
            bool hasChildren = false;
            DIR *sub = opendir(full.c_str());
            if (sub)
            {
                struct dirent *sdp;
                while ((sdp = readdir(sub)) != nullptr)
                {
                    std::string sname = sdp->d_name;
                    if (sname == "." || sname == "..")
                        continue;
                    struct stat sst;
                    if (stat((full + "/" + sname).c_str(), &sst) == 0 && S_ISDIR(sst.st_mode))
                    {
                        hasChildren = true;
                        break;
                    }
                }
                closedir(sub);
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
        closedir(dirp);
    }

    Json::Value data;
    data["path"] = rel;
    data["nodes"] = nodes;

    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}