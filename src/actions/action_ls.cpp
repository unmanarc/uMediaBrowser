#include "../webclienthdlr.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

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
        logRpc(Mantids30::Program::Logs::WARNING, "R/LS-404: path='%s' not found or not a directory", path.c_str());
        Json::Value d;
        writeJSON(false, "Directory not found", d);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    const std::string rel = dirInfo.relativePath.empty() ? "/" : dirInfo.relativePath;

    // Compute the parent path.
    std::string parent = "/";
    if (rel != "/" && !rel.empty())
    {
        std::string p = fs::path(rel).parent_path().string();
        if (!p.empty() && p != ".")
            parent = p;
    }

    Json::Value data;
    data["path"] = rel;
    data["parent"] = parent;
    data["has_parent"] = (parent != "/");

    Json::Value flags;
    flags["uploads"] = webClientParameters.uploads;
    flags["overwrite"] = webClientParameters.overwrite;
    flags["targz"] = webClientParameters.targz;
    flags["execute"] = webClientParameters.execute;
    flags["version"] = webClientParameters.softwareVersion;
    data["flags"] = flags;

    data["upload_here"] = !fs::exists(dirInfo.fullPath + ".dontupload");

    Json::Value items(Json::arrayValue);

    std::error_code ec;
    if (!fs::is_directory(dirInfo.fullPath, ec) || ec)
    {
        logRpc(Mantids30::Program::Logs::WARNING, "R/LS-403: path='%s' not accessible (%s)", path.c_str(), ec.message().c_str());
        writeJSON(false, "Folder not accessible", data);
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
    }

    std::vector<Json::Value> vec;
    for (auto it = fs::directory_iterator(dirInfo.fullPath, ec); !ec && it != fs::directory_iterator(); it.increment(ec))
    {
        const fs::directory_entry &entry = *it;
        const std::string name = entry.path().filename().string();

        std::error_code sec;
        const fs::file_status st = entry.status(sec);
        if (sec)
            continue;

        const fs::file_type type = st.type();
        if (type == fs::file_type::unknown)
            continue;
        const bool isDir = (type == fs::file_type::directory);

        // st_uid / st_gid are not exposed by std::filesystem; keep a plain stat for them.
        struct stat64 rawStat;
        if (stat64(entry.path().c_str(), &rawStat) != 0)
            rawStat.st_uid = rawStat.st_gid = 0;

        const fs::perms perms = st.permissions();
        char chmodval[16];
        snprintf(chmodval, sizeof(chmodval), "%o", static_cast<int>(static_cast<int>(perms) & static_cast<int>(fs::perms::all)));

        Json::Value item;
        item["name"] = name;
        item["type"] = isDir ? "dir" : "file";
        item["size"] = (Json::Int64) rawStat.st_size;
        item["size_fancy"] = isDir ? "" : getFancySize(static_cast<size_t>(rawStat.st_size));
        item["mtime"] = (Json::Int64) rawStat.st_mtime;
        item["uid"] = (Json::UInt) rawStat.st_uid;
        item["gid"] = (Json::UInt) rawStat.st_gid;
        item["perms"] = chmodval;
        const fs::perms others = perms & fs::perms::others_all;
        item["warning"] = (others == (fs::perms::others_write | fs::perms::others_exec) || others == (fs::perms::others_read | fs::perms::others_write | fs::perms::others_exec));
        item["executable"] = ((perms & fs::perms::owner_exec) != fs::perms::none);

        vec.push_back(item);
    }

    // Folders first, then case-insensitive name order.
    std::sort(vec.begin(),
              vec.end(),
              [](const Json::Value &a, const Json::Value &b)
              {
                  const bool ad = a["type"].asString() == "dir";
                  const bool bd = b["type"].asString() == "dir";
                  if (ad != bd)
                      return ad; // dirs come first
                  std::string an = a["name"].asString(), bn = b["name"].asString();
                  std::transform(an.begin(), an.end(), an.begin(), ::tolower);
                  std::transform(bn.begin(), bn.end(), bn.begin(), ::tolower);
                  return an < bn;
              });

    for (const Json::Value &v : vec)
        items.append(v);

    data["items"] = items;

    logRpc(Mantids30::Program::Logs::INFO, "R/LS: path='%s' -> %u items (dirs first)", rel.c_str(), (unsigned) vec.size());
    if (webClientParameters.debug && ec)
        logRpc(Mantids30::Program::Logs::DEBUG, "R/LS-DIRERR: path='%s' %s", path.c_str(), ec.message().c_str());

    writeJSON(true, "", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}