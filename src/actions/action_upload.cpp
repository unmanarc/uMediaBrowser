#include "../webclienthdlr.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Memory/streamable_file.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <fstream>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Network::Protocol::MIME;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Program;

static bool resolveUploadPath0NE(const std::string &fileName, const std::string &serverDir, WebClientHdlr::LocalRequestedFileInfo *info)
{
    if (!info)
        return false;

    info->reset();

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

void cbFileTransferMIMEHeaderReady(void *obj, const std::string &partName, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage)
{
    webClientParams *webClientParameters = (webClientParams *)obj;
    auto fileName = partMessage->getHeader()->getOptionByName("Content-Disposition")->getSubVar("filename");

    HTTPv1_Server::LocalRequestedFileInfo reqInfo;

    if (!access((webClientParameters->uploadDirBase + "/.dontupload").c_str(), F_OK))
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS-DIR: %s", webClientParameters->uploadDirBase.c_str());
        return;
    }

    if (!access((webClientParameters->uploadDirBase + "/.uploadname").c_str(), R_OK))
    {
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

    if (WebClientHdlr::resolveLocalFilePathFromURI0E("/" + fileName, webClientParameters->uploadDirBase, &reqInfo))
    {
        if (boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.dontupload")
            || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.uploadname")
            || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.upload"))
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        if (webClientParameters->overwrite)
        {
            auto file = std::make_shared<StreamableFile>();
            if (file->open(reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0700) >= 0)
            {
                partMessage->getContent()->replaceContentContainer(file);
                webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-OVERW-OK: %s", reqInfo.relativePath.c_str());
                webClientParameters->uploadOK = true;
            }
            else
            {
                webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS1: %s", reqInfo.relativePath.c_str());
                file.reset();
            }
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-OVERWRITE: %s", reqInfo.relativePath.c_str());
        }
    }
    else if (resolveUploadPath0NE(fileName, webClientParameters->uploadDirBase, &reqInfo))
    {
        if (boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.dontupload")
            || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.uploadname"))
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        auto file = std::make_shared<StreamableFile>();
        if (file->open(reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT, 0700) >= 0)
        {
            partMessage->getContent()->replaceContentContainer(file);
            webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-CREATE-OK: %s", reqInfo.relativePath.c_str());
            webClientParameters->uploadOK = true;
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS2: %s", reqInfo.relativePath.c_str());
            file.reset();
        }
    }
    else if (reqInfo.isTransversal)
    {
        webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-TRANSVERSAL2: %s", fileName.c_str());
    }
    else
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP,"","", "", "fileServer", 65535, "R/UPLOAD-FAIL: %s", fileName.c_str());
    }
}

Status::Code WebClientHdlr::action_upload()
{
    Json::Value data;
    data["uploaded"] = webClientParameters.uploadOK;
    writeJSON(webClientParameters.uploadOK, webClientParameters.uploadOK? "" : "Upload not completed", data);
    return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
}
