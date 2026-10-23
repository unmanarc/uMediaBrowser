#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Memory/streamable_process.h>
#include <Mantids30/Program_Logs/loglevels.h>

#include <string>
#include <vector>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Helpers;

Status::Code WebClientHdlr::action_archive()
{
    if (!webClientParameters.targz)
    {
        Json::Value d;
        writeJSON(false, "Archive downloads are disabled", d);
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
    }

    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string filesParam = vars->getStringValue("files");
    std::string fmt = vars->getStringValue("fmt");
    if (fmt.empty())
        fmt = "tar.gz";

    std::vector<std::string> names = ActionCommon::splitComma(filesParam);
    if (names.empty())
    {
        Json::Value d;
        writeJSON(false, "No files selected", d);
        return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
    }

    // Resolve every requested path (traversal protected) and split it into
    // parent directory + entry name, so the archive contains only the
    // requested items (not their full absolute host path).
    std::string parent;
    std::vector<std::string> bases;
    for (const std::string &f : names)
    {
        LocalRequestedFileInfo info;
        if (!resolveExistingPath(f, info))
        {
            Json::Value d;
            writeJSON(false, "Invalid path: " + f, d);
            return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
        }
        if (info.relativePath == "/" || info.relativePath.empty())
        {
            Json::Value d;
            writeJSON(false, "Document root cannot be archived", d);
            return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
        }

        std::string full = info.fullPath;
        while (full.size() > 1 && full.back() == '/')
            full.pop_back();   // directories resolve with a trailing slash
        const size_t slash = full.find_last_of('/');
        std::string pdir, base;
        if (slash == std::string::npos)
        {
            pdir = ".";
            base = full;
        }
        else
        {
            base = full.substr(slash + 1);
            pdir = (slash == 0) ? "/" : full.substr(0, slash);
        }
        // The UI only selects items within a single directory, so all items
        // share the same parent; use the first one as the archive base.
        if (parent.empty())
            parent = pdir;
        bases.push_back(base);
    }

    Helpers::AppSpawn *spawner = new Helpers::AppSpawn();
    std::string ext = "tar.gz";

    if (fmt == "zip")
    {
        ext = "zip";
        // Info-ZIP `zip` has no "--chdir"; wrap it in a sh subshell that
        // changes into the parent dir. Paths are passed positionally ($1, $@)
        // so arbitrary file names are safe from shell injection.
        spawner->setExec("/bin/sh");
        spawner->addArgument("-c");
        spawner->addArgument("cd \"$1\" && shift && exec /usr/bin/zip -r - \"$@\"");
        spawner->addArgument("sh");
        spawner->addArgument(parent);
    }
    else if (fmt == "tar")
    {
        ext = "tar";
        spawner->setExec("/usr/bin/tar");
        spawner->addArgument("cf");
        spawner->addArgument("-");
        spawner->addArgument("-C");
        spawner->addArgument(parent);
    }
    else // tar.gz (default)
    {
        ext = "tar.gz";
        spawner->setExec("/usr/bin/tar");
        spawner->addArgument("czf");
        spawner->addArgument("-");
        spawner->addArgument("-C");
        spawner->addArgument(parent);
    }

    for (const std::string &b : bases)
        spawner->addArgument(b);

    serverResponse.contentType = "application/octet-stream";

    const std::string downloadName = "archive." + ext;
    serverResponse.headers.replace("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");

    if (spawner->spawnProcess(true, false))
    {
        auto streamableP = std::make_shared<StreamableProcess>(spawner);
        serverResponse.setContentDataStreamer(streamableP);
    }
    else
    {
        webClientParameters.rpcLog->log(Mantids30::Program::Logs::WARNING, clientRequest.networkClientInfo.REMOTE_ADDR,"","", "", "fileServer", 2048, "R/ARCHIVE-FAIL: %s", fmt.c_str());
        delete spawner;
    }

    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}