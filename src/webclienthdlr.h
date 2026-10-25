#pragma once

#include <Mantids30/Memory/b_chunks.h>
#include <Mantids30/Memory/b_mem.h>
#include <Mantids30/Protocol_HTTP/httpv1_server.h>
#include <Mantids30/Program_Logs/rpclog.h>
#include <json/json.h>

struct webClientParams
{
    webClientParams()
    {
        execute = false;
        rpcLog = nullptr;
        targz = false;
        uploads = false;
        overwrite = false;
        uploadOK = false;
    }

    bool execute,targz,uploads,overwrite;
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

    bool containOnlyAllowedChars(const std::string &str);

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
    Mantids30::Network::Protocol::HTTP::Status::Code action_exec();

    webClientParams webClientParameters;
};



