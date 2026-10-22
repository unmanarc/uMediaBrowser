#ifndef WEBCLIENTHANDLER_H
#define WEBCLIENTHANDLER_H

#include <Mantids30/Memory/b_chunks.h>
#include <Mantids30/Memory/b_mem.h>
#include <Mantids30/Protocol_HTTP/httpv1_server.h>
#include <Mantids30/Program_Logs/rpclog.h>

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

};

class WebClientHdlr : public Mantids30::Network::Protocol::HTTP::HTTPv1_Server
{
public:
    using LocalRequestedFileInfo = Mantids30::Network::Protocol::HTTP::HTTPv1_Server::LocalRequestedFileInfo;

    WebClientHdlr(const std::shared_ptr<Mantids30::Memory::Streams::StreamableObject> &sock);
    ~WebClientHdlr() override;

    void setWebClientParameters(const webClientParams &newWebClientParameters);

    /*
     * Public wrapper used by the MIME upload callback (the base class version
     * is a protected static function).
     */
    static bool resolveLocalFilePathFromURI0E(const std::string &uri, std::string serverDir, LocalRequestedFileInfo *info)
    {
        return HTTPv1_Server::resolveLocalFilePathFromURI0E(uri, serverDir, info);
    }

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
    void generateIndexOf(const LocalRequestedFileInfo &fileInfo);
    void generateTarGz(const LocalRequestedFileInfo &fileInfo);

    std::string getFancySize(size_t  size);
    webClientParams webClientParameters;


};



#endif // WEBCLIENTHANDLER_H
