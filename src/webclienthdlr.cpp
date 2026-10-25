#include "webclienthdlr.h"
#include "html_errors.h"
#include "html_index_app.h"
#include <Mantids30/Helpers/random.h>
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <string>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Network::Protocol::MIME;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Program;

// Forward declaration of the upload MIME callback (defined in action_upload.cpp).
void cbFileTransferMIMEHeaderReady(void *obj, const std::string &partName, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage);

WebClientHdlr::WebClientHdlr(const std::shared_ptr<StreamableObject> &sock)
    : HTTPv1_Server(sock)
{}

WebClientHdlr::~WebClientHdlr() {}

bool WebClientHdlr::checkCSRFTokenOnPOST()
{
    if (clientRequest.requestLine.getHTTPMethod() == "POST")
    {
        const std::string headerToken = clientRequest.headers.getOptionRawStringByName("X-CSRFToken");
        const std::string cookieToken = clientRequest.getCookie("CSRFToken");

        // Both tokens must be present and match.
        if (headerToken.empty() || cookieToken.empty() || headerToken != cookieToken)
        {
            logRpc(Logs::ERROR, "R/POST-FAIL-CSRF: %s", clientRequest.getURI().c_str());
            return false;
        }
    }
    return true;
}

bool WebClientHdlr::onHTTPClientHeadersReceived()
{
    // When behind a proxy (--use-x-real-ip), the X-Real-IP header is authoritative for all request logs.
    if (webClientParameters.useXRealIP)
    {
        std::string realIP = clientRequest.headers.getOptionRawStringByName("X-Real-IP");
        boost::trim(realIP);
        if (!realIP.empty())
        {
            clientRequest.networkClientInfo.setClientInformation(realIP, clientRequest.networkClientInfo.isSecure, clientRequest.networkClientInfo.tlsCommonName);
        }
    }

    if (clientRequest.content.getContainerType() == Content::ContainerType::MIME)
        clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback());

    if (!checkCSRFTokenOnPOST())
        return true;

    auto httpVars_GET = clientRequest.getVarsBySource(HTTP::Source::GET);
    const std::string act = httpVars_GET->getStringValue("act");

    if (clientRequest.requestLine.getHTTPMethod() == "POST" && act == "upload" && webClientParameters.canUpload)
    {
        HTTPv1_Server::LocalRequestedFileInfo reqInfo;
        const std::string path = httpVars_GET->getStringValue("path");

        if (resolveExistingPath(path.empty() ? "/" : path, reqInfo) && reqInfo.isDirectory)
        {
            webClientParameters.uploadDirBase = reqInfo.fullPath;
            webClientParameters.remoteIP = clientRequest.networkClientInfo.REMOTE_ADDR;
            clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback(&cbFileTransferMIMEHeaderReady, &webClientParameters));
            logRpc(Logs::INFO, "R/UPLOAD-REQ: %s", reqInfo.fullPath.c_str());
        }
        else
        {
            logRpc(Logs::CRITICAL, "R/UPLOAD-TRANSVERSAL1: %s", path.c_str());
            return false;
        }
    }

    return true;
}

Mantids30::Network::Protocol::HTTP::Status::Code WebClientHdlr::respond(Mantids30::Network::Protocol::HTTP::Status::Code ret, const std::string &uri)
{
    const long long elapsedMs = requestElapsedMs();
    const bool ok = (ret == HTTP::Status::Code::S_200_OK);

    // One line per request: status, method, URI and elapsed time.
    logRpc(ok ? Logs::INFO : Logs::WARNING,
           "R/%03d:%s: %s (%lld ms)",
           static_cast<int>(ret),
           clientRequest.requestLine.getHTTPMethod().c_str(),
           uri.c_str(),
           elapsedMs);

    if (webClientParameters.debug && !clientRequest.getURI().empty())
        logRpc(Logs::DEBUG, "R/FULLURI: %s", clientRequest.getURI().c_str());

    return ret;
}

long long WebClientHdlr::requestElapsedMs() const
{
    if (reqStart.time_since_epoch().count() == 0)
        return 0;
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - reqStart).count();
}

void WebClientHdlr::createCSRFTokenCookie()
{
    if (clientRequest.requestLine.getHTTPMethod() == "GET")
    {
        Headers::Cookie val;
        val.value = Helpers::Random::createRandomString(32);
        val.secure = clientRequest.networkClientInfo.isSecure;
        val.httpOnly = false;
        val.path = "/";
        val.setAsSessionCookie();
        val.sameSitePolicy = Headers::Cookie::SameSitePolicy::STRICT;
        serverResponse.setCookie("CSRFToken", val);
    }
}

void WebClientHdlr::writeJSON(const bool ok, const std::string &error, const Json::Value &data)
{
    serverResponse.contentType = "application/json; charset=UTF-8";

    Json::Value root;
    root["ok"] = ok;
    root["error"] = error;
    root["data"] = data;

    Json::StreamWriterBuilder w;
    w["indentation"] = "";
    w["emitUTF8"] = true;
    getResponseContentStreamableObject()->writeString(Json::writeString(w, root));
}

bool WebClientHdlr::resolveExistingPath(const std::string &uriStylePath, LocalRequestedFileInfo &info)
{
    return resolveLocalFilePathFromURI0E(uriStylePath.empty() ? "/" : uriStylePath, webClientParameters.httpDocumentRootDir, &info);
}

bool WebClientHdlr::resolveNewChildPath(const std::string &parentUri, const std::string &childName, LocalRequestedFileInfo &info)
{
    info.reset();

    if (childName.empty() || childName == "." || childName == ".." || childName.find('/') != std::string::npos || childName.find('\\') != std::string::npos)
        return false;

    // Parent must exist and be a directory.
    if (!resolveExistingPath(parentUri.empty() ? "/" : parentUri, info) || !info.isDirectory)
        return false;

    // Build the combined URI and let 0NE resolve it (normalizes symlinks, flags transversal, requires non-existence).
    const std::string parentRel = info.relativePath.empty() ? "/" : info.relativePath;
    const std::string uri = (parentRel.back() == '/' ? parentRel : parentRel + "/") + childName;

    LocalRequestedFileInfo candidate;
    if (!HTTPv1_Server::resolveLocalFilePathFromURI0NE(uri, webClientParameters.httpDocumentRootDir, &candidate) || candidate.isTransversal || candidate.exists)
        return false;

    info = candidate;
    info.isDirectory = true; // 0NE does not stat (the path does not exist yet).
    return true;
}

Status::Code WebClientHdlr::onHTTPClientContentReceived()
{
    HTTP::Status::Code ret = HTTP::Status::Code::S_404_NOT_FOUND;

    reqStart = std::chrono::steady_clock::now();

    if (!checkCSRFTokenOnPOST())
        return HTTP::Status::Code::S_401_UNAUTHORIZED;

    if (!webClientParameters.pass.empty())
    {
        if (!clientRequest.basicAuth.isEnabled || clientRequest.basicAuth.password != webClientParameters.pass || clientRequest.basicAuth.username != webClientParameters.user)
        {
            logRpc(Logs::ERROR, "R/AUTH-FAIL: user='%s' method=%s",
                   clientRequest.basicAuth.username.c_str(),
                   clientRequest.requestLine.getHTTPMethod().c_str());
            getResponseContentStreamableObject()->writeString(HTML_401);
            serverResponse.sWWWAuthenticateRealm = "Authentication Required";
            return respond(HTTP::Status::Code::S_401_UNAUTHORIZED, clientRequest.getURI().c_str());
        }
    }

    createCSRFTokenCookie();

    auto HTTPVars_GET = clientRequest.getVarsBySource(HTTP::Source::GET);
    const std::string method = clientRequest.requestLine.getHTTPMethod();
    const std::string act = HTTPVars_GET->getStringValue("act");

    // Request-entry line: what arrived before dispatch.
    logRpc(Logs::DEBUG, "R/REQ: %s act='%s' uri=%s",
           method.c_str(),
           act.c_str(),
           clientRequest.getURI().c_str());

    if (clientRequest.content.getContainerType() == Content::ContainerType::MIME && method == "POST" && act == "upload" && webClientParameters.canUpload)
    {
        logRpc(Logs::INFO, "R/UPLOAD: %s", webClientParameters.uploadOK ? "completed" : "incomplete (no part processed)");
        Json::Value data;
        data["uploaded"] = webClientParameters.uploadOK;
        writeJSON(webClientParameters.uploadOK, webClientParameters.uploadOK ? "" : "Upload not completed", data);
        return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
    }

    if (act == "ls")
        return action_ls();
    if (act == "tree")
        return action_tree();
    if (act == "download")
        return action_download();
    if (act == "archive")
        return action_archive();
    if (act == "upload")
        return action_upload();
    if (act == "mkdir")
        return action_mkdir();
    if (act == "delete")
        return action_delete();
    if (act == "thumb")
        return action_thumb();
    if (act == "detail")
        return action_detail();
    if (act == "exec")
        return action_exec();

    if (method == "GET")
    {
        serverResponse.contentType = "text/html; charset=UTF-8";
        getResponseContentStreamableObject()->writeString(buildHTMLApp(webClientParameters.appTitle));
        return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
    }

    getResponseContentStreamableObject()->writeString(HTML_404);
    logRpc(Logs::WARNING, "R/404-NOACT: act='%s' method=%s", act.c_str(), method.c_str());
    return respond(ret, clientRequest.getURI().c_str());
}

bool WebClientHdlr::containOnlyAllowedChars(const std::string &str)
{
    static const boost::regex allowed("^[a-zA-Z0-9.,!@_-+=:/]*$");
    return boost::regex_match(str, allowed);
}

std::string WebClientHdlr::getFancySize(size_t value)
{
    const char *szNames[] = {"B", "kB", "MB", "GB", "TB"};

    size_t pos = 0, remainder = 0;

    for (; value >= 1024 && pos < (sizeof szNames / sizeof *szNames); value /= 1024)
    {
        remainder = (value % 1024);
        pos++;
    }

    char output[64];

    if (remainder)
        sprintf(output, "%.1f%s", (double) value + ((double) remainder / (double) 1024), szNames[pos]);
    else
        sprintf(output, "%zu%s", value, szNames[pos]);
    return std::string(output);
}

void WebClientHdlr::setWebClientParameters(const webClientParams &newWebClientParameters)
{
    webClientParameters = newWebClientParameters;
}
