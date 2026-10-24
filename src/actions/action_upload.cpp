#include "../webclienthdlr.h"
#include <Mantids30/Memory/streamable_file.h>
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Network::Protocol::MIME;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Program;


void cbFileTransferMIMEHeaderReady(void *obj, const std::string &partName, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage)
{
    webClientParams *webClientParameters = (webClientParams *) obj;
    auto fileName = partMessage->getHeader()->getOptionByName("Content-Disposition")->getSubVar("filename");

    HTTPv1_Server::LocalRequestedFileInfo reqInfo;

    if (fs::exists(webClientParameters->uploadDirBase + "/.dontupload"))
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS-DIR: %s", webClientParameters->uploadDirBase.c_str());
        return;
    }

    if (fs::exists(webClientParameters->uploadDirBase + "/.uploadname"))
    {
        std::ifstream file;
        std::string line;
        file.open((webClientParameters->uploadDirBase + "/.uploadname").c_str());
        if (file)
        {
            if (std::getline(file, line))
            {
                boost::trim(line);
                fileName = line;
            }
            file.close();
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-UPLOADNAME-PERM: %s", webClientParameters->uploadDirBase.c_str());
            return;
        }
    }

    if (HTTPv1_Server::resolveLocalFilePathFromURI0E("/" + fileName, webClientParameters->uploadDirBase, &reqInfo))
    {
        // File exist
        if (   boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.dontupload")
            || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.uploadname")
            || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.upload"))
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        if (webClientParameters->overwrite)
        {
            auto file = std::make_shared<StreamableFile>();
            if (file->open(reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0700) >= 0)
            {
                partMessage->getContent()->replaceContentContainer(file);
                webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-OVERW-OK: %s", reqInfo.relativePath.c_str());
                webClientParameters->uploadOK = true;
            }
            else
            {
                webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS1: %s", reqInfo.relativePath.c_str());
                file.reset();
            }
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-OVERWRITE: %s", reqInfo.relativePath.c_str());
        }
    }
    // TODO: URL Encoded file name (and filename field name)
    else if (HTTPv1_Server::resolveLocalFilePathFromURI0NE( "/" + fileName, webClientParameters->uploadDirBase, &reqInfo))
    {
        // File does not exist.
        if (boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.dontupload") || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.uploadname"))
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        auto file = std::make_shared<StreamableFile>();
        if (file->open(reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT, 0700) >= 0)
        {
            partMessage->getContent()->replaceContentContainer(file);
            webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-CREATE-OK: %s", reqInfo.relativePath.c_str());
            webClientParameters->uploadOK = true;
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS2: %s", reqInfo.relativePath.c_str());
            file.reset();
        }
    }
    else if (reqInfo.isTransversal)
    {
        // Transversal: outside the directory...
        webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-TRANSVERSAL2: %s", fileName.c_str());
    }
    else
    {
        // ??
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL: %s", fileName.c_str());
    }
}

Status::Code WebClientHdlr::action_upload()
{
    Json::Value data;
    data["uploaded"] = webClientParameters.uploadOK;
    writeJSON(webClientParameters.uploadOK, webClientParameters.uploadOK ? "" : "Upload not completed", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}
