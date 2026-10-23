#include "../webclienthdlr.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;

Status::Code WebClientHdlr::action_ls()
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

    // Compute the parent path.
    std::string parent = "/";
    if (rel != "/" && !rel.empty())
    {
        size_t pos = rel.find_last_of('/');
        if (pos == 0)
            parent = "/";
        else if (pos != std::string::npos)
            parent = rel.substr(0, pos);
    }

    Json::Value data;
    data["path"] = rel;
    data["parent"] = parent;

    Json::Value flags;
    flags["uploads"] = webClientParameters.uploads;
    flags["overwrite"] = webClientParameters.overwrite;
    flags["targz"] = webClientParameters.targz;
    flags["execute"] = webClientParameters.execute;
    flags["version"] = webClientParameters.softwareVersion;
    data["flags"] = flags;

    data["upload_here"] = (access((dirInfo.fullPath + ".dontupload").c_str(), F_OK) != 0);

    Json::Value items(Json::arrayValue);

    DIR *dirp = opendir(dirInfo.fullPath.c_str());
    if (!dirp)
    {
        writeJSON(false, "Folder not accessible", data);
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
    }

    struct dirent *dp;
    std::vector<Json::Value> vec;
    while ((dp = readdir(dirp)) != nullptr)
    {
        std::string name = dp->d_name;
        if (name == "." || name == "..")
            continue;

        std::string full = dirInfo.fullPath + "/" + name;
        struct stat64 stats;
        if (stat64(full.c_str(), &stats) != 0)
            continue;

        bool isDir = S_ISDIR(stats.st_mode);
        char chmodval[16];
        snprintf(chmodval, sizeof(chmodval), "%o", (int)(stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)));

        Json::Value item;
        item["name"] = name;
        item["type"] = isDir? "dir" : "file";
        item["size"] = (Json::Int64)stats.st_size;
        item["size_fancy"] = isDir? "" : getFancySize((size_t)stats.st_size);
        item["mtime"] = (Json::Int64)stats.st_mtime;
        item["uid"] = (Json::UInt)stats.st_uid;
        item["gid"] = (Json::UInt)stats.st_gid;
        item["perms"] = chmodval;
        item["warning"] = ((stats.st_mode & S_IRWXO) == 7 || (stats.st_mode & S_IRWXO) == 6);
        item["executable"] = ((stats.st_mode & S_IXUSR) != 0);

        vec.push_back(item);
    }
    closedir(dirp);

    // Folders first, then case-insensitive name order.
    std::sort(vec.begin(), vec.end(), [](const Json::Value &a, const Json::Value &b){
        const bool ad = a["type"].asString() == "dir";
        const bool bd = b["type"].asString() == "dir";
        if (ad != bd)
            return ad;   // dirs come first
        std::string an = a["name"].asString(), bn = b["name"].asString();
        std::transform(an.begin(), an.end(), an.begin(), ::tolower);
        std::transform(bn.begin(), bn.end(), bn.begin(), ::tolower);
        return an < bn;
    });

    for (const Json::Value &v : vec)
        items.append(v);

    data["items"] = items;

    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}