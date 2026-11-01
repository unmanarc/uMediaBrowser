#pragma once

#include <Mantids30/Memory/b_chunks.h>
#include <Mantids30/Memory/b_mem.h>
#include <Mantids30/Protocol_HTTP/httpv1_server.h>
#include <Mantids30/Program_Logs/rpclog.h>
#include <json/json.h>

#include <chrono>
#include <string>
#include <utility>

struct webClientParams
{
    webClientParams()
    {
        rpcLog = nullptr;
        canArchive = false;
        canUpload = false;
        canOverwrite = false;
        uploadOK = false;
        debug = false;
        useXRealIP = false;
    }

    bool canArchive,canUpload,canOverwrite;
    bool debug; ///< Verbose logging (-d/--debug)
    bool useXRealIP; ///< Use X-Real-IP header for client IP in logs (behind proxy)
    std::string softwareVersion;
    std::string httpDocumentRootDir;
    Mantids30::Program::Logs::RPCLog * rpcLog;
    std::string user,pass;

    std::string uploadDirBase;
    std::string remoteIP;
    bool uploadOK;
    std::string appTitle;

};

class WebClientHdlr : public Mantids30::Network::Protocol::HTTP::HTTPv1_Server
{
public:
    using LocalRequestedFileInfo = Mantids30::Network::Protocol::HTTP::HTTPv1_Server::LocalRequestedFileInfo;

    WebClientHdlr(const std::shared_ptr<Mantids30::Memory::Streams::StreamableObject> &sock);
    ~WebClientHdlr() override;

    void setWebClientParameters(const webClientParams &newWebClientParameters);

    // Non-owning access for the upload MIME callback (lives in action_upload.cpp).
    webClientParams *getWebClientParametersPtr() { return &webClientParameters; }

    /**
     * @brief logRpc Log a per-client request event through the shared RPCLog.
     * The IP and module are filled automatically; `fmt` is a printf-style format
     * (may be omitted when there are no variadic args).
     * DEBUG/DEBUG1 lines only print when the logger was put in debug mode.
     */
    void logRpc(Mantids30::Program::Logs::LogLevel level, const char *fmt)
    {
        if (webClientParameters.rpcLog)
            webClientParameters.rpcLog->log(level, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 4096, "%s", fmt);
    }

    template <typename... Args>
    void logRpc(Mantids30::Program::Logs::LogLevel level, const char *fmt, Args &&...args)
    {
        if (webClientParameters.rpcLog)
            webClientParameters.rpcLog->log(level, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 4096, fmt, std::forward<Args>(args)...);
    }

    // Milliseconds elapsed since the current request started (0 before reqStart is set).
    long long requestElapsedMs() const;

protected:
    /**
     * @brief onHTTPClientHeadersReceived Process Client Options (eg. check if it's a file uploader)
     * @return true to continue receiving the data, false to terminate the connection.
     */
    bool onHTTPClientHeadersReceived() override;

    /**
     * @brief onHTTPClientContentReceived Process web client request
     * @return http response code.
     */
    Mantids30::Network::Protocol::HTTP::Status::Code onHTTPClientContentReceived() override;

private:
    Mantids30::Network::Protocol::HTTP::Status::Code respond(Mantids30::Network::Protocol::HTTP::Status::Code ret , const std::string &uri);

    void createCSRFTokenCookie();
    bool checkCSRFTokenOnPOST();

    std::string getFancySize(size_t  size);

    // Write a JSON envelope: { "ok": bool, "error": string, "data": ... }
    void writeJSON(const bool ok, const std::string &error, const Json::Value &data);

    // Resolve an existing URI-style path (eg. "/a/b") against the document root.
    bool resolveExistingPath(const std::string &uriStylePath, LocalRequestedFileInfo &info);
    // Resolve a not-yet-existing child inside an existing parent directory.
    bool resolveNewChildPath(const std::string &parentUri, const std::string &childName, LocalRequestedFileInfo &info);

    // Actions (each implemented in its own src/actions/*.cpp):
    Mantids30::Network::Protocol::HTTP::Status::Code action_ls();
    Mantids30::Network::Protocol::HTTP::Status::Code action_tree();
    Mantids30::Network::Protocol::HTTP::Status::Code action_download();
    Mantids30::Network::Protocol::HTTP::Status::Code action_archive();
    Mantids30::Network::Protocol::HTTP::Status::Code action_upload();
    Mantids30::Network::Protocol::HTTP::Status::Code action_mkdir();
    Mantids30::Network::Protocol::HTTP::Status::Code action_delete();
    Mantids30::Network::Protocol::HTTP::Status::Code action_thumb();
    Mantids30::Network::Protocol::HTTP::Status::Code action_detail();

    webClientParams webClientParameters;
    std::chrono::steady_clock::time_point reqStart;
};



