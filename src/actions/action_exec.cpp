#include "../html_errors.h"
#include "../webclienthdlr.h"
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Memory/streamable_process.h>
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Helpers;
using namespace Mantids30::Program;

Status::Code WebClientHdlr::action_exec()
{
    if (!webClientParameters.execute)
    {
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string file = vars->getStringValue("file");

    LocalRequestedFileInfo fileInfo;
    if (!resolveExistingPath(file, fileInfo) || fileInfo.isDirectory)
    {
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    // Only executable regular files may be executed (CGI-like).
    std::error_code ec;
    const fs::file_status st = fs::status(fileInfo.fullPath, ec);
    if (ec || st.type() != fs::file_type::regular || (st.permissions() & fs::perms::owner_exec) == fs::perms::none)
    {
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    std::string composedArgumentList, composedEnviromentList;
    Helpers::AppSpawn *spawner = new Helpers::AppSpawn();
    if (!spawner->setExec(fileInfo.fullPath))
    {
        delete spawner;
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    spawner->addEnvironment(std::string("REMOTE_ADDR=") + clientRequest.networkClientInfo.REMOTE_ADDR);
    spawner->addEnvironment(std::string("REQUEST_METHOD=") + clientRequest.requestLine.getHTTPMethod());

    auto postVars = clientRequest.getVarsBySource(HTTP::Source::POST);
    for (const auto &key : postVars->getKeysList())
    {
        if (!containOnlyAllowedChars(key) || !containOnlyAllowedChars(postVars->getStringValue(key)))
            continue;
        spawner->addEnvironment("POST_" + key + "=" + postVars->getStringValue(key));
        composedEnviromentList += (composedEnviromentList.empty() ? "" : " ") + std::string("POST_") + key;
    }

    auto getVars = clientRequest.getVarsBySource(HTTP::Source::GET);
    for (uint32_t i = 1; i < 1024 && getVars->getValue("a" + std::to_string(i)); i++)
    {
        spawner->addArgument(getVars->getStringValue("a" + std::to_string(i)));
        composedArgumentList += (composedArgumentList.empty() ? "" : " ") + getVars->getStringValue("a" + std::to_string(i));
    }

    webClientParameters.rpcLog->log(Logs::DEBUG,
                                    clientRequest.networkClientInfo.REMOTE_ADDR,
                                    "",
                                    "",
                                    "",
                                    "fileServer",
                                    2048,
                                    "D/EXEC: %s (args: %s, env: %s)",
                                    fileInfo.fullPath.c_str(),
                                    composedArgumentList.c_str(),
                                    composedEnviromentList.c_str());

    if (spawner->spawnProcess(true, false))
    {
        webClientParameters.rpcLog->log(Logs::INFO,
                                        clientRequest.networkClientInfo.REMOTE_ADDR,
                                        "",
                                        "",
                                        "",
                                        "fileServer",
                                        2048,
                                        "E/EXEC-OK: %s (args: %s, env: %s)",
                                        fileInfo.relativePath.c_str(),
                                        composedArgumentList.c_str(),
                                        composedEnviromentList.c_str());
        auto streamableP = std::make_shared<StreamableProcess>(spawner);
        serverResponse.setContentDataStreamer(streamableP);
        detectContentTypeFromFilePath(fileInfo.relativePath);
    }
    else
    {
        webClientParameters.rpcLog
            ->log(Logs::WARNING, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 2048, "E/EXEC-FAIL: %s (args: %s)", fileInfo.relativePath.c_str(), composedArgumentList.c_str());
        delete spawner;
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_500_INTERNAL_SERVER_ERROR, clientRequest.getURI().c_str());
    }

    return respond(HTTP::Status::Code::S_200_OK, fileInfo.relativePath);
}