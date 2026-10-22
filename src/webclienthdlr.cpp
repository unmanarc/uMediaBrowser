#include "webclienthdlr.h"
#include "html_errors.h"
#include "html_index_css.h"
#include "html_index_upload.h"
#include "html_index_templates.h"
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <boost/range/end.hpp>
#include <Mantids30/Memory/b_mmap.h>
#include <Mantids30/Protocol_HTTP/streamencoder_url.h>
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Helpers/random.h>
#include <Mantids30/Memory/streamable_process.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <Mantids30/Memory/streamable_file.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <fstream>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Network::Protocol::MIME;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Program;

// TODO: HTML for uploads
// TODO: null stream por default

WebClientHdlr::WebClientHdlr(const std::shared_ptr<StreamableObject> &sock) : HTTPv1_Server(sock)
{
}

WebClientHdlr::~WebClientHdlr()
{
}

bool WebClientHdlr::checkCSRFTokenOnPOST()
{
    if ( clientRequest.requestLine.getHTTPMethod() == "POST" )
    {
        // All POST should be checked with AntiCSRF token

        if (        clientRequest.headers.getOptionRawStringByName("X-CSRFToken")
                    !=  clientRequest.getCookie("CSRFToken"))
        {
            // All POST requests should come with anti-csrf token...
            webClientParameters.rpcLog->log(Logs::ERROR, clientRequest.networkClientInfo.REMOTE_ADDR,"","", "", "fileServer", 65535, "R/POST-FAIL-CSRF: %s",clientRequest.getURI().c_str());
            // Ret false
            return false;
        }
    }
    // TOKEN OK.
    return true;
}

/**
 * @brief resolveUploadPath0NE Resolve a non-existent upload destination.
 *
 * Mantids30's resolveLocalFilePathFromURI0NE() requires realpath() to succeed,
 * so it can't be used for a file that still doesn't exist. This helper keeps
 * the original behavior and the same traversal protections for uploads.
 */
static bool resolveUploadPath0NE(const std::string &fileName, const std::string &serverDir, WebClientHdlr::LocalRequestedFileInfo *info)
{
    if (!info)
        return false;

    info->reset();

    // Only allow plain file names inside the upload directory.
    if (fileName.empty() || fileName == "." || fileName == ".."
        || fileName.find('/') != std::string::npos
        || fileName.find('\\') != std::string::npos)
    {
        info->isTransversal = true;
        return false;
    }

    char *resolvedDir = realpath(serverDir.c_str(), nullptr);
    if (!resolvedDir)
        return false;

    std::string fullPath(resolvedDir);
    free(resolvedDir);

    if (fullPath.empty() || fullPath.back() != '/')
        fullPath += '/';

    info->fullPath = fullPath + fileName;
    info->relativePath = "/" + fileName;
    info->exists = false;
    return true;
}

void cbFileTransferMIMEHeaderReady(void *obj, const std::string & partName, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage)
{
    webClientParams * webClientParameters = (webClientParams *)obj;
    auto fileName =partMessage->getHeader()->getOptionByName("Content-Disposition")->getSubVar("filename");
    HTTPv1_Server::LocalRequestedFileInfo reqInfo;

    if (!access((webClientParameters->uploadDirBase + "/.dontupload").c_str(),F_OK))
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS-DIR: %s", webClientParameters->uploadDirBase.c_str());
        return;
    }
    if (!access((webClientParameters->uploadDirBase + "/.uploadname").c_str(),R_OK))
    {
        // Access new upload name:
        std::ifstream file;
        std::string line;

        file.open((webClientParameters->uploadDirBase + "/.uploadname").c_str());
        if (file)
        {
            if (getline(file, line))
            {
                while (line.back() == '\n' || line.back() == '\r')
                    line.pop_back();
                fileName = line;
            }
            file.close();
        }
        else
            return;
    }

    // Check if file exist:
    if (WebClientHdlr::resolveLocalFilePathFromURI0E( "/" + fileName , webClientParameters->uploadDirBase, &reqInfo ))
    {
        if ( boost::ends_with( boost::to_lower_copy(reqInfo.fullPath), "/.dontupload")  ||
             boost::ends_with( boost::to_lower_copy(reqInfo.fullPath), "/.uploadname") ||
             boost::ends_with( boost::to_lower_copy(reqInfo.fullPath), "/.upload")
             )
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        if ( webClientParameters->overwrite )
        {
            // Allowed to overwrite...
            // if exist, and there is no transversal attack... overwrite...
            auto file = std::make_shared<StreamableFile>();
            if (file->open( reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0700 )>=0)
            {
                partMessage->getContent()->replaceContentContainer(file);
                webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-OVERW-OK: %s", reqInfo.relativePath.c_str());
                webClientParameters->uploadOK = true;
            }
            else
            {
                webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS1: %s",reqInfo.relativePath.c_str());
                file.reset();
            }
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-OVERWRITE: %s",reqInfo.relativePath.c_str());
        }
    }
    // TODO: URL Encoded file name (and filename field name)
    else if (resolveUploadPath0NE(fileName, webClientParameters->uploadDirBase, &reqInfo))
    {
        if ( boost::ends_with( boost::to_lower_copy(reqInfo.fullPath), "/.dontupload")  ||
             boost::ends_with( boost::to_lower_copy(reqInfo.fullPath), "/.uploadname")
             )
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        // Yes or Not allowed to overwrite and file does not exist...
        auto file = std::make_shared<StreamableFile>();
        if (file->open( reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT, 0700 )>=0)
        {
            partMessage->getContent()->replaceContentContainer(file);
            webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-CREATE-OK: %s", reqInfo.relativePath.c_str());
            webClientParameters->uploadOK = true;
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS2: %s",reqInfo.relativePath.c_str());
            file.reset();
        }
    }
    else if ( reqInfo.isTransversal )
    {
        webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-TRANSVERSAL2: %s", fileName.c_str());
    }
    else
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL: %s", fileName.c_str());
    }
}

bool WebClientHdlr::onHTTPClientHeadersReceived()
{
    // Reset the callback.
    if (clientRequest.content.getContainerType() == Content::ContainerType::MIME)
        clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady( MIME::MIME_Message::MIMECallback() );

    if (!checkCSRFTokenOnPOST())
        return true;

    if ( clientRequest.requestLine.getHTTPMethod() == "POST" )
    {
        // All POST should be checked with AntiCSRF token
        if (     boost::ends_with(clientRequest.requestLine.getURI(), "/.upload") &&
                 webClientParameters.uploads == true
                 )
        {
            // Reset:

            HTTPv1_Server::LocalRequestedFileInfo reqInfo;

            std::string uri = clientRequest.requestLine.getURI();

            if (HTTPv1_Server::resolveLocalFilePathFromURI0E( uri, webClientParameters.httpDocumentRootDir, &reqInfo ))
            {
                // not allowed to upload here.
                webClientParameters.rpcLog->log(Logs::ERROR, clientRequest.networkClientInfo.REMOTE_ADDR,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-NOTALLOWED: %s",clientRequest.getURI().c_str());
                return false;
            }

            uri.pop_back();//u
            uri.pop_back();//p
            uri.pop_back();//l
            uri.pop_back();//o
            uri.pop_back();//a
            uri.pop_back();//d

            webClientParameters.remoteIP = clientRequest.networkClientInfo.REMOTE_ADDR;

            if (HTTPv1_Server::resolveLocalFilePathFromURI0E( uri, webClientParameters.httpDocumentRootDir, &reqInfo ))
            {
                if (reqInfo.isDirectory)
                {
                    webClientParameters.uploadDirBase = reqInfo.fullPath;
                    clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady( MIME::MIME_Message::MIMECallback(&cbFileTransferMIMEHeaderReady, &webClientParameters) );
                    webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"","", "", "fileServer", 65535, "R/UPLOAD-REQ: %s",clientRequest.getURI().c_str());
                }
            }
            else
            {
                webClientParameters.rpcLog->log(Logs::CRITICAL, webClientParameters.remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-TRANSVERSAL1: %s", uri.c_str());
                return false;
            }
        }
    }

    return true;
}


Mantids30::Network::Protocol::HTTP::Status::Code WebClientHdlr::respond( Mantids30::Network::Protocol::HTTP::Status::Code ret, const std::string &uri )
{
    webClientParameters.rpcLog->log(ret == HTTP::Status::Code::S_200_OK? Logs::INFO :  Logs::WARNING,
                                    clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "R/%03d:%s: %s",
                                    static_cast<int>(ret),
                                    clientRequest.requestLine.getHTTPMethod().c_str(),
                                    uri.c_str());
    return ret;
}

void WebClientHdlr::createCSRFTokenCookie()
{
    if ( clientRequest.requestLine.getHTTPMethod() == "GET" )
    {
        Headers::Cookie val;
        val.value = Helpers::Random::createRandomString(32);

        // Secure if the communication is HTTPS... (important to avoid various mitm attacks to the CSRF)
        val.secure = clientRequest.networkClientInfo.isSecure;

        // Readable by the javascript:
        val.httpOnly = false;
        val.path = "/";
        val.setAsSessionCookie();
        val.sameSitePolicy = Headers::Cookie::SameSitePolicy::STRICT;
        serverResponse.setCookie("CSRFToken",val);
    }
}


Status::Code WebClientHdlr::onHTTPClientContentReceived()
{
    LocalRequestedFileInfo fileInfo;
    HTTP::Status::Code ret  = HTTP::Status::Code::S_404_NOT_FOUND;

    if (!checkCSRFTokenOnPOST())
        return HTTP::Status::Code::S_401_UNAUTHORIZED;

    if (!webClientParameters.pass.empty())
    {
        if (    !clientRequest.basicAuth.isEnabled ||
                clientRequest.basicAuth.password != webClientParameters.pass ||
                clientRequest.basicAuth.username!= webClientParameters.user)
        {
            getResponseContentStreamableObject()->writeString(HTML_401);
            serverResponse.sWWWAuthenticateRealm = "Authentication Required";
            return respond( HTTP::Status::Code::S_401_UNAUTHORIZED, clientRequest.getURI().c_str() );
        }
    }

    // Create CSRF Token on the cookie
    createCSRFTokenCookie();

    if (clientRequest.content.getContainerType() == Content::ContainerType::MIME)
    {
        if (     clientRequest.requestLine.getHTTPMethod() == "POST" &&
                 boost::ends_with(clientRequest.requestLine.getURI(), "/.upload") &&
                 webClientParameters.uploads == true
                 )
        {
            std::string uri = clientRequest.requestLine.getURI();
            uri.pop_back();//u
            uri.pop_back();//p
            uri.pop_back();//l
            uri.pop_back();//o
            uri.pop_back();//a
            uri.pop_back();//d
            getResponseContentStreamableObject()->writeString(webClientParameters.uploadOK?"true":"false");
            return respond( HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str() );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    if ((       resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, ".html",webClientParameters.execute) ||
                resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, "index.html",webClientParameters.execute) ||
                resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, "/index.html",webClientParameters.execute) ||
                resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, "",webClientParameters.execute)
                ) && !fileInfo.isDirectory)
    {
        // Evaluate...

        if (fileInfo.isExecutable)
        {
            if (clientRequest.requestLine.getHTTPMethod() == "POST" || clientRequest.requestLine.getHTTPMethod() == "GET")
            {
                // Execute...
                ret = HTTP::Status::Code::S_200_OK;

                std::string composedArgumentList, composedEnviromentList;
                Helpers::AppSpawn * spawner = new Helpers::AppSpawn();
                spawner->setExec( fileInfo.fullPath );

                spawner->addEnvironment(std::string("REMOTE_ADDR=") + clientRequest.networkClientInfo.REMOTE_ADDR);
                spawner->addEnvironment(std::string("REQUEST_METHOD=") + clientRequest.requestLine.getHTTPMethod());

                auto postVars = clientRequest.getVarsBySource(HTTP::Source::POST);
                for (const auto & key : postVars->getKeysList())
                {
                    // Security: Don't insert dangerous chars... (it should allow base64, emails, etc)
                    if (!containOnlyAllowedChars(key) || !containOnlyAllowedChars(postVars->getStringValue(key))) continue;

                    spawner->addEnvironment("POST_" + key + "=" + postVars->getStringValue(key));
                    composedEnviromentList += (composedEnviromentList.empty()?"": " ") + std::string("POST_") + key;
                }

                auto getVars = clientRequest.getVarsBySource(HTTP::Source::GET);
                for ( uint32_t i=1; i<1024 && getVars->getValue("a" + std::to_string(i)); i++)
                {
                    spawner->addArgument(getVars->getStringValue("a" + std::to_string(i)));
                    composedArgumentList += (composedArgumentList.empty()?"": " ") + getVars->getStringValue(std::string("a" + std::to_string(i)));
                }

                webClientParameters.rpcLog->log(Logs::DEBUG, clientRequest.networkClientInfo.REMOTE_ADDR,"","", "", "fileServer", 2048, "D/EXEC,%03d: %s (args: %s, env: %s)",
                                                static_cast<int>(ret),fileInfo.fullPath.c_str(), composedArgumentList.c_str(), composedEnviromentList.c_str());
                if (spawner->spawnProcess(true,false))
                {
                    webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "E/EXEC-OK,%03d: %s (args: %s, env: %s)",
                                                    static_cast<int>(ret),fileInfo.relativePath.c_str(), composedArgumentList.c_str(), composedEnviromentList.c_str());
                    auto streamableP = std::make_shared<StreamableProcess>(spawner);
                    serverResponse.setContentDataStreamer(streamableP);
                    detectContentTypeFromFilePath(fileInfo.relativePath);
                }
                else
                {
                    webClientParameters.rpcLog->log(Logs::WARNING, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "E/EXEC-FAIL,%03d: %s (args: %s)",static_cast<int>(ret),fileInfo.relativePath.c_str(), composedArgumentList.c_str());
                    delete spawner;
                }
            }
        }
        else
        {
            //Serve as it comes...
            ret = HTTP::Status::Code::S_200_OK;
            //            webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "R/%03d: %s",static_cast<int>(ret),fileInfo.relativePath.c_str());
            webClientParameters.rpcLog->log(Logs::DEBUG, clientRequest.networkClientInfo.REMOTE_ADDR,"","", "", "fileServer", 2048, "D/LOCAL,%03d: %s",static_cast<int>(ret),fileInfo.fullPath.c_str());
        }


    }
    else if (resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo) && fileInfo.isDirectory)
    {
        if (fileInfo.relativePath == "") fileInfo.relativePath = "/";
        ret = HTTP::Status::Code::S_200_OK;

        auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);

        if (vars->getStringValue("action") == "targz" && webClientParameters.targz)
        {
            webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "A/TARGZ-DIR,%03d: %s",static_cast<int>(ret),fileInfo.relativePath.c_str());
            generateTarGz(fileInfo);
        }
        else
        {
            //webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "I/DIR,%03d: %s",static_cast<int>(ret),fileInfo.relativePath.c_str());
            generateIndexOf(fileInfo);
        }
    }
    else
    {
        getResponseContentStreamableObject()->writeString(HTML_404);
    }

    if (fileInfo.relativePath.empty())
        return respond(ret,clientRequest.getURI());
    else
        return respond(ret,fileInfo.relativePath);
}

bool WebClientHdlr::containOnlyAllowedChars(const std::string & str)
{
    for (ssize_t i=0;i<str.size();i++)
    {
        if ( !(            (str.at(i)>='a' && str.at(i)<='z')
                           ||     (str.at(i)>='A' && str.at(i)<='Z')
                           ||     (str.at(i)>='0' && str.at(i)<='9')
                           ||      str.at(i)=='.' ||  str.at(i)==',' ||  str.at(i)=='!' ||  str.at(i)=='@' ||  str.at(i)=='_' ||  str.at(i)=='-' ||  str.at(i)=='+' ||  str.at(i)==':' ||  str.at(i)=='=' ||  str.at(i)=='/'
                           )
             )
        {
            return false;
        }
    }
    return true;

}
void WebClientHdlr::generateTarGz(const LocalRequestedFileInfo &fileInfo)
{
    serverResponse.contentType = "application/octet-stream";

    std::string composedArgumentList;
    Helpers::AppSpawn * spawner = new Helpers::AppSpawn();

    spawner->setExec( "/usr/bin/tar" );
    spawner->addArgument( "czf" ); // Compress (gzip), and put in file format.
    spawner->addArgument( "-" ); // Output to stdout (here)
    spawner->addArgument( fileInfo.fullPath ); // Our magic dir...

    if (spawner->spawnProcess(true,false))
    {
        auto streamableP = std::make_shared<StreamableProcess>(spawner);
        serverResponse.setContentDataStreamer(streamableP);
    }
    else
    {
        webClientParameters.rpcLog->log(Logs::WARNING, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "R/EXEC-FAIL: tar czvf");
        delete spawner;
    }
}

void WebClientHdlr::generateIndexOf(const LocalRequestedFileInfo & fileInfo)
{
    serverResponse.contentType = "text/html";


    getResponseContentStreamableObject()->strPrintf(HTML_INDEX_HEADER,
                                          htmlEncode(fileInfo.relativePath).c_str(),
                                          HTML_CSS_PAGE, HTML_JS_UPLOAD,
                                          htmlEncode(fileInfo.relativePath).c_str(),
                                          webClientParameters.targz? HTML_TARGZ_LINK :"",
                                          webClientParameters.uploads?HTML_UPLOAD_FORM:"");

    DIR *dirp;
    struct dirent *dp;

    if ((dirp = opendir(fileInfo.fullPath.c_str())) == NULL)
    {
        getResponseContentStreamableObject()->writeString(HTML_INDEX_FOLDER_INACCESSIBLE);
    }
    else
    {
        while ((dp = readdir(dirp)) != NULL)
        {
            struct stat64 stats;
            stat64(       std::string(fileInfo.fullPath + std::string("/") + dp->d_name).c_str()
                          , &stats);


            time_t t = stats.st_mtime;
            struct tm lt;
            localtime_r(&t, &lt);
            char timebuf_mtime[128];
            strftime(timebuf_mtime, sizeof(timebuf_mtime), "%c", &lt);


            int statchmod = stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
            char chmodval[128];

            if (    (stats.st_mode & S_IRWXO) == 7 ||
                    (stats.st_mode & S_IRWXO) == 6
                    )
            {
                snprintf(chmodval, sizeof(chmodval), HTML_CHMOD_WARNING, statchmod);
            }
            else
            {
                snprintf(chmodval, sizeof(chmodval), HTML_CHMOD_NORMAL, statchmod);
            }

            getResponseContentStreamableObject()->strPrintf(HTML_INDEX_ROW,
                        S_ISDIR(stats.st_mode)? HTML_ICON_FOLDER_PREFIX : HTML_ICON_FILE_PREFIX,
                        (Memory::Streams::Encoders::URL::encodeURLStr(dp->d_name) + (S_ISDIR(stats.st_mode)?"/":"")).c_str(),
                        htmlEncode(dp->d_name).c_str(),
                        S_ISDIR(stats.st_mode)? HTML_ICON_FOLDER_SUFFIX : HTML_ICON_FILE_SUFFIX,
                        getFancySize(stats.st_size).c_str(),
                        timebuf_mtime,
                        stats.st_uid,
                        stats.st_gid,
                        chmodval
                        );
        }
        closedir(dirp);
        getResponseContentStreamableObject()->writeString(HTML_INDEX_TABLE_CLOSE);
    }



    getResponseContentStreamableObject()->strPrintf(HTML_INDEX_FOOTER, webClientParameters.softwareVersion.c_str());

}



std::string WebClientHdlr::getFancySize(size_t value)
{
    const char *szNames[] = { "B", "kB", "MB", "GB", "TB" };

    size_t pos = 0, remainder = 0;

    for (; value >= 1024 && pos < (sizeof szNames / sizeof *szNames); value /= 1024)
    {
        remainder = (value % 1024);
        pos++;
    }

    char output[64];

    if (remainder)
        sprintf(output, "%.1f%s\n", (double)value + ((double)remainder / (double)1024), szNames[pos]);
    else
        sprintf(output, "%zu%s\n", value, szNames[pos]);
    return std::string(output);
}

void WebClientHdlr::setWebClientParameters(const webClientParams &newWebClientParameters)
{
    webClientParameters = newWebClientParameters;
}

