#include "../webclienthdlr.h"
#include "action_common.h"
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Memory/streamable_process.h>
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Helpers;
using namespace Mantids30::Helpers::JSON;

namespace
{
    // Reduce a name to a safe set of characters for use in a download
    // filename. Keeps [A-Za-z0-9._-], collapses runs of dots (never '..'),
    // strips leading dots/dashes and trailing dots, and caps the length.
    std::string sanitizeName(const std::string &in)
    {
        std::string out;
        out.reserve(in.size());
        for (char c : in)
        {
            if (isalnum((unsigned char) c) || c == '.' || c == '_' || c == '-')
            {
                if (c == '.')
                {
                    // Avoid ".." and a leading dot.
                    if (!out.empty() && out.back() == '.')
                        continue;
                    if (out.empty())
                        continue;
                }
                out.push_back(c);
            }
        }
        while (!out.empty() && (out.back() == '.' || out.back() == '-'))
            out.pop_back();
        if (out.size() > 80)
            out.resize(80);
        while (!out.empty() && (out.back() == '.' || out.back() == '-'))
            out.pop_back(); // trimming may have re-exposed a trailing dot
        return out;
    }

    // Longest character-wise common prefix of every entry in the list.
    std::string commonPrefix(const std::vector<std::string> &names)
    {
        if (names.empty())
            return {};
        std::string pref = names[0];
        for (std::size_t i = 1; i < names.size() && !pref.empty(); ++i)
        {
            std::size_t n = 0;
            while (n < pref.size() && n < names[i].size() && pref[n] == names[i][n])
                ++n;
            pref.resize(n);
        }
        return pref;
    }
}

Status::Code WebClientHdlr::action_archive()
{
    if (!webClientParameters.canArchive)
    {
        writeJSON(false, "Archive downloads are disabled", boost::json::value{});
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
        logRpc(Mantids30::Program::Logs::WARNING, "R/TAR-400: no files selected");
        writeJSON(false, "No files selected", boost::json::value{});
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
            logRpc(Mantids30::Program::Logs::WARNING, "R/TAR-400: invalid path='%s'", f.c_str());
            writeJSON(false, "Invalid path: " + f, boost::json::value{});
            return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
        }
        if (info.relativePath == "/" || info.relativePath.empty())
        {
            logRpc(Mantids30::Program::Logs::WARNING, "R/TAR-400: refusing to archive document root");
            writeJSON(false, "Document root cannot be archived", boost::json::value{});
            return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
        }

        std::string full = info.fullPath;
        while (full.size() > 1 && full.back() == '/')
            full.pop_back(); // directories resolve with a trailing slash
        const fs::path p(full);
        std::string pdir = p.parent_path().string();
        std::string base = p.filename().string();
        if (pdir.empty())
            pdir = (full[0] == '/') ? "/" : ".";
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

    // Derive the download name from the selected items instead of a fixed
    // "archive": a single item is named after itself (extension dropped);
    // multiple items share their longest common prefix. Anything that is not
    // a safe filename falls back to "archive".
    std::string baseName;
    if (bases.size() == 1)
        baseName = sanitizeName(bases[0]);
    else
    {
        std::vector<std::string> san;
        san.reserve(bases.size());
        for (const std::string &b : bases)
            san.push_back(sanitizeName(b));
        baseName = sanitizeName(commonPrefix(san));
        // A prefix that is empty, too short, or ends in a dot would produce a
        // poor download name; fall back to the generic one.
        if (baseName.empty() || baseName.size() < 3)
            baseName.clear();
    }
    if (baseName.empty())
        baseName = "archive";

    const std::string downloadName = baseName + "." + ext;
    serverResponse.headers.replace("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");

    if (spawner->spawnProcess(true, false))
    {
        auto streamableP = std::make_shared<StreamableProcess>(spawner);
        serverResponse.setContentDataStreamer(streamableP);
        logRpc(Mantids30::Program::Logs::INFO, "R/TAR: %u file(s) -> %s (fmt=%s)", (unsigned) bases.size(), downloadName.c_str(), fmt.c_str());
    }
    else
    {
        logRpc(Mantids30::Program::Logs::WARNING, "R/TAR-FAIL: could not spawn %s for %u file(s)", fmt.c_str(), (unsigned) bases.size());
        delete spawner;
    }

    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}