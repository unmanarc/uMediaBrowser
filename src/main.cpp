#include <Mantids30/Net_Sockets/socket_tcp.h>
#include <Mantids30/Program_Logs/applog.h>
#include <Mantids30/Program_Service/application.h>
#ifdef WITH_SSL_SUPPORT
#include <Mantids30/Net_Sockets/socket_tls.h>
#endif
#include <Mantids30/Memory/a_bool.h>
#include <Mantids30/Memory/a_uint16.h>
#include <Mantids30/Net_Sockets/acceptor_multithreaded.h>

#include <filesystem>
#include <fstream>
#include <inttypes.h>
#include <sys/time.h>

#include "config.h"
#include "favicon.h"
#include "html_errors.h"
#include "webclienthdlr.h"

using namespace Mantids30;
using namespace Mantids30::Memory;
using namespace Mantids30::Program;
namespace fs = std::filesystem;

class UMediaBrowser : public Mantids30::Program::Application
{
public:
    static std::shared_ptr<Mantids30::Memory::Containers::B_MEM> favicon;
    static time_t staticContentTime;

    UMediaBrowser() { multiThreadedAcceptor = std::make_shared<Network::Sockets::Acceptors::MultiThreaded>(); }

    void _shutdown() override { log->log0(__func__, Logs::INFO, "SIGTERM received."); }

    void _initvars(int argc, char *argv[], Arguments::GlobalArguments *globalArguments) override
    {
        globalArguments->setInifiniteWaitAtEnd(true);

        favicon = std::make_shared<Mantids30::Memory::Containers::B_MEM>(favicon2_ico, favicon2_ico_len);
        staticContentTime = time(nullptr);

        /////////////////////////
        globalArguments->setVersion(std::stoi(PROJECT_VER_MAJOR), std::stoi(PROJECT_VER_MINOR), std::stoi(PROJECT_VER_PATCH), "a");

        webClientParameters.softwareVersion = globalArguments->getVersion();
        globalArguments->softwareLicense = "AGPL";
        globalArguments->softwareDescription = std::string(PROJECT_DESCRIPTION);
        globalArguments->addAuthor({"Aarón Mizrachi", "aaron@unmanarc.com"});

        globalArguments->addCommandLineOption("Server Options",
                                              'x',
                                              "can-execute",
#ifndef _WIN32
                                              "Execute any eXecutable marked file and get the output, ATT: disable can-archive if you want to keep your script sources private",
#else
                                              "Execute any .exe/.bat file and get the output, ATT: disable can-archive if you want to keep your script sources private",
#endif
                                              "false",
                                              Memory::Abstract::Var::Type::BOOL);

        globalArguments->addCommandLineOption("Server Options", 'g', "can-archive", "Allow to get server directories as tar.gz files (on-the-fly)", "false", Memory::Abstract::Var::Type::BOOL);
        globalArguments->addCommandLineOption("Server Options", 'u', "can-upload", "Allow file uploads", "false", Memory::Abstract::Var::Type::BOOL);
        globalArguments->addCommandLineOption("Server Options", 'o', "can-overwrite", "Allow overwrite on file uploads and/or delete files", "false", Memory::Abstract::Var::Type::BOOL);

        globalArguments->addCommandLineOption("HTTP Options", 'r', "rootdir", "HTTP Document Root Directory", ".", Memory::Abstract::Var::Type::STRING);
        globalArguments->addCommandLineOption("HTTP Options", 'l', "lport", "Local HTTP Port", "8001", Memory::Abstract::Var::Type::UINT16);
        globalArguments->addCommandLineOption("HTTP Options", '4', "ipv4", "Use only IPv4", "true", Memory::Abstract::Var::Type::BOOL);
        globalArguments->addCommandLineOption("HTTP Options", 'a', "laddr", "Listen Address", "*", Memory::Abstract::Var::Type::STRING);
        globalArguments->addCommandLineOption("HTTP Options", 0, "title", "Application title shown in browser tab and toolbar", "", Memory::Abstract::Var::Type::STRING);
        globalArguments->addCommandLineOption("HTTP Options", 't', "threads", "Max Concurrent Connections (Threads)", "1024", Memory::Abstract::Var::Type::UINT16);
        globalArguments->addCommandLineOption("HTTP Options", 0, "use-x-real-ip", "Use X-Real-IP header for client IP in logs instead of the source IP (behind proxy)", "false", Memory::Abstract::Var::Type::BOOL);

        globalArguments->addCommandLineOption("HTTP Security", 'p', "passfile", "HTTP User/Pass File", "", Memory::Abstract::Var::Type::STRING);

        globalArguments->addCommandLineOption("Other Options", 's', "sys", "Journalctl Log Mode (don't print colors or dates)", "false", Memory::Abstract::Var::Type::BOOL);
        globalArguments->addCommandLineOption("Other Options", 'd', "debug", "Verbose debug logging (full URIs, per-entry detail)", "false", Memory::Abstract::Var::Type::BOOL);

#ifdef WITH_SSL_SUPPORT
        globalArguments
            ->addCommandLineOption("TLS Options", 0, "cafile", "X.509 Certificate Authority Path (if not defined, then without client authentication)", "", Memory::Abstract::Var::Type::STRING);
        globalArguments->addCommandLineOption("TLS Options", 'k', "keyfile", "X.509 Private Key Path (if not defined, then HTTP)", "", Memory::Abstract::Var::Type::STRING);
        globalArguments->addCommandLineOption("TLS Options", 'c', "certfile", "X.509 Certificate Path (if not defined, then HTTP)", "", Memory::Abstract::Var::Type::STRING);
#endif
    }

    bool _config(int argc, char *argv[], Arguments::GlobalArguments *globalArguments) override
    {
#ifdef WITH_SSL_SUPPORT
        Mantids30::Network::Sockets::Socket_TLS::prepareTLS();
#endif
        bool configUseFancy = !globalArguments->getCommandLineOptionBooleanValue("sys");
        const bool debugMode = globalArguments->getCommandLineOptionBooleanValue("debug");
        fprintf(stderr, "# Arguments: %s\n", globalArguments->getCurrentProgramOptionsValuesAsBashLine().c_str());

        log = new Logs::AppLog();
        log->enableEmptyFieldLogging = true;
        log->userFieldMinWidth = 1;
        log->enableAttributeNameLogging = false;
        log->enableColorLogging = configUseFancy;
        log->enableDateLogging = configUseFancy;
        log->moduleFieldMinWidth = 36;
        log->setDebug(debugMode);

        webClientParameters.rpcLog = new Logs::RPCLog();
        webClientParameters.rpcLog->enableEmptyFieldLogging = false;
        webClientParameters.rpcLog->userFieldMinWidth = 1;
        webClientParameters.rpcLog->enableAttributeNameLogging = false;
        webClientParameters.rpcLog->enableColorLogging = configUseFancy;
        webClientParameters.rpcLog->enableDateLogging = configUseFancy;
        webClientParameters.rpcLog->moduleFieldMinWidth = 36;
        webClientParameters.rpcLog->setDebug(debugMode);

        std::string passFile = globalArguments->getCommandLineOptionValue("passfile")->toString();

        if (!passFile.empty())
        {
            std::ifstream file(passFile);
            if (file.is_open())
            {
                if (!std::getline(file, webClientParameters.user))
                {
                    log->log0(__func__, Logs::CRITICAL, "Password File '%s' require at least 2 lines [user/pass)", passFile.c_str());
                    exit(-14);
                }
                if (!std::getline(file, webClientParameters.pass))
                {
                    log->log0(__func__, Logs::CRITICAL, "Password File '%s' require at least 2 lines (user/pass]", passFile.c_str());
                    exit(-15);
                }
                file.close();
            }
            else
            {
                log->log0(__func__, Logs::CRITICAL, "Failed to open user/password file '%s'", passFile.c_str());
                exit(-13);
            }
        }

        listenAddress = globalArguments->getCommandLineOptionValue("laddr")->toString();
        webClientParameters.httpDocumentRootDir = globalArguments->getCommandLineOptionValue("rootdir")->toString();
        listenPort = static_cast<uint16_t>(std::stoul(globalArguments->getCommandLineOptionValue("lport")->toString()));
        webClientParameters.canExecute = globalArguments->getCommandLineOptionBooleanValue("can-execute");
        webClientParameters.canArchive = globalArguments->getCommandLineOptionBooleanValue("can-archive");
        webClientParameters.canUpload = globalArguments->getCommandLineOptionBooleanValue("can-upload");
        webClientParameters.canOverwrite = globalArguments->getCommandLineOptionBooleanValue("can-overwrite");
        webClientParameters.debug = globalArguments->getCommandLineOptionBooleanValue("debug");
        webClientParameters.useXRealIP = globalArguments->getCommandLineOptionBooleanValue("use-x-real-ip");
        webClientParameters.appTitle = globalArguments->getCommandLineOptionValue("title")->toString();

        bool configUseIPv4 = globalArguments->getCommandLineOptionBooleanValue("ipv4");
        auto configThreads = static_cast<uint32_t>(std::stoul(globalArguments->getCommandLineOptionValue("threads")->toString()));

#ifdef WITH_SSL_SUPPORT
        std::string certfile = globalArguments->getCommandLineOptionValue("certfile")->toString();
        std::string keyfile = globalArguments->getCommandLineOptionValue("keyfile")->toString();
        std::string cafile = globalArguments->getCommandLineOptionValue("cafile")->toString();
#endif

        auto socketTCP = std::make_shared<Network::Sockets::Socket_TCP>();
        std::shared_ptr<Network::Sockets::Socket_Stream> acceptorSocket = socketTCP;
#ifdef WITH_SSL_SUPPORT
        auto socketTLS = std::make_shared<Network::Sockets::Socket_TLS>();

        // Set the SO default security level:
        socketTLS->tlsKeys.setSecurityLevel(-1);

        if ((keyfile.size() && certfile.empty()) || (keyfile.empty() && certfile.size()))
        {
            log->log0(__func__, Logs::CRITICAL, "X.509 Private Key and Cert File should be configured in pairs.");
            exit(-13);
            return false;
        }

        if (keyfile.size())
        {
            if (access(keyfile.c_str(), R_OK))
            {
                log->log0(__func__, Logs::CRITICAL, "X.509 Private Key not found.");
                exit(-10);
                return false;
            }
            socketTLS->tlsKeys.loadPrivateKeyFromPEMFile(keyfile.c_str());
        }
        if (certfile.size())
        {
            if (access(certfile.c_str(), R_OK))
            {
                log->log0(__func__, Logs::CRITICAL, "X.509 Certificate not found.");
                exit(-11);
                return false;
            }
            socketTLS->tlsKeys.loadPublicKeyFromPEMFile(certfile.c_str());
        }
        if (cafile.size())
        {
            if (!fs::exists(cafile))
            {
                log->log0(__func__, Logs::CRITICAL, "X.509 Certificate Authority not found.");
                exit(-12);
                return false;
            }
            socketTLS->tlsKeys.loadCAFromPEMFile(cafile);
        }
        if (keyfile.size() && certfile.size())
            acceptorSocket = socketTLS;
#endif
        acceptorSocket->setUseIPv6(!configUseIPv4);
        if (!acceptorSocket->listenOn(listenPort, listenAddress.c_str()))
        {
            log->log0(__func__, Logs::CRITICAL, "Unable to listen at %s:%" PRIu16, listenAddress.c_str(), listenPort);
            exit(-20);
            return false;
        }
        multiThreadedAcceptor->addAcceptorSocket(acceptorSocket);
        multiThreadedAcceptor->callbacks.setAllContexts(this);
        multiThreadedAcceptor->callbacks.onClientConnected = _callbackOnConnect;
        multiThreadedAcceptor->callbacks.onProtocolInitializationFailure = _callbackOnInitFailed;
        multiThreadedAcceptor->callbacks.onClientAcceptTimeoutOccurred = _callbackOnTimeOut;
        multiThreadedAcceptor->parameters.setMaxConcurrentClients(configThreads);

        return true;
    }

    int _start(int argc, char *argv[], Arguments::GlobalArguments *globalArguments) override
    {
        std::error_code ec;
        const fs::path rp = fs::canonical(webClientParameters.httpDocumentRootDir, ec);

        if (ec)
        {
            log->log0(__func__, Logs::CRITICAL, "Invalid Document Root Directory.");
            exit(-1);
        }
        multiThreadedAcceptor->startInBackground();

        log->log0(__func__, Logs::INFO, "Web Server Loaded @%s:%" PRIu16, listenAddress.c_str(), listenPort);
        log->log0(__func__, Logs::INFO, "Document root: %s", rp.c_str());
        log->log0(__func__, Logs::INFO, "Features: can-upload=%d can-overwrite=%d can-archive=%d can-execute=%d",
                  webClientParameters.canUpload,
                  webClientParameters.canOverwrite,
                  webClientParameters.canArchive,
                  webClientParameters.canExecute);
        log->log0(__func__, Logs::INFO, "Auth: %s | Debug logging: %s",
                  webClientParameters.pass.empty() ? "disabled" : "basic",
                  webClientParameters.debug ? "enabled" : "disabled");

        return 0;
    }

    const webClientParams &getWebClientParameters() const;

private:
    /**
     * callback when connection is fully established
     */
    static void _callbackOnConnect(void *, const std::shared_ptr<Network::Sockets::Socket_Stream> &);
    /**
     * callback when protocol initialization failed (like bad X.509 on TLS)
     */
    static void _callbackOnInitFailed(void *, const std::shared_ptr<Network::Sockets::Socket_Stream> &);
    /**
     * callback when timed out (all the thread queues are saturated) (this callback is called from acceptor thread, you should use it very quick)
     */
    static void _callbackOnTimeOut(void *, const std::shared_ptr<Network::Sockets::Socket_Stream> &);

    std::string listenAddress;
    uint16_t listenPort;
    Logs::AppLog *log;
    webClientParams webClientParameters;

    std::shared_ptr<Network::Sockets::Acceptors::MultiThreaded> multiThreadedAcceptor;
};

void UMediaBrowser::_callbackOnConnect(void *obj, const std::shared_ptr<Network::Sockets::Socket_Stream> &s)
{
    std::string tlsCN;
    UMediaBrowser *webServer = (UMediaBrowser *) obj;

    bool isSecure = s->isSecure();
    if (isSecure)
    {
#ifdef WITH_SSL_SUPPORT
        Network::Sockets::Socket_TLS *tlsSock = (Network::Sockets::Socket_TLS *) s.get();
        tlsCN = tlsSock->getTLSPeerCN();
#endif
    }

    // Prepare the web services handler.
    auto webHandler = std::make_shared<WebClientHdlr>(s);

    Mantids30::Network::Protocol::HTTP::HTTPv1_Server::StaticContentElement faviconElement;
    faviconElement.setData(favicon, staticContentTime);

    // Mantids30 resolves in-memory static content without the leading slash.
    // Register both variants to keep the original URL mapping working.
    webHandler->addStaticContent("favicon.ico", faviconElement);
    webHandler->addStaticContent("/favicon.ico", faviconElement);

    char inetAddr[INET6_ADDRSTRLEN];
    s->getRemotePair(inetAddr);

    webServer->log->log1("web", inetAddr, Logs::INFO, "C/CONNECT: %s%s%s",
                         inetAddr,
                         isSecure ? " [TLS]" : "",
                         tlsCN.empty() ? "" : (" CN=" + tlsCN).c_str());

    webHandler->clientRequest.networkClientInfo.setClientInformation(inetAddr, isSecure, tlsCN);
    webHandler->setWebClientParameters(webServer->getWebClientParameters());

    // Handle the Web Client here:
    Memory::Streams::Parser::ParseResult err;
    webHandler->parseObject(&err);
}

void UMediaBrowser::_callbackOnInitFailed(void *obj, const std::shared_ptr<Network::Sockets::Socket_Stream> &s)
{
    char inetAddr[INET6_ADDRSTRLEN];
    s->getRemotePair(inetAddr);
    ((UMediaBrowser *) obj)->log->log1("web", inetAddr, Logs::WARNING, "C/INIT-FAIL: protocol initialization failed (eg. bad TLS handshake)");
}

void UMediaBrowser::_callbackOnTimeOut(void *obj, const std::shared_ptr<Network::Sockets::Socket_Stream> &s)
{
    char inetAddr[INET6_ADDRSTRLEN];
    s->getRemotePair(inetAddr);
    ((UMediaBrowser *) obj)->log->log1("web", inetAddr, Logs::WARNING, "C/TIMEOUT: no request received in time, all thread queues saturated, sending 503");
    s->writeString(HTML_503_RESPONSE);
}

const webClientParams &UMediaBrowser::getWebClientParameters() const
{
    return webClientParameters;
}

int main(int argc, char *argv[])
{
    UMediaBrowser *main = new UMediaBrowser;
    return StartApplication(argc, argv, main);
}

std::shared_ptr<Mantids30::Memory::Containers::B_MEM> UMediaBrowser::favicon = nullptr;
time_t UMediaBrowser::staticContentTime = 0;
