#include "../webclienthdlr.h"
#include "action_common.h"
#include "../html_errors.h"
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <Mantids30/Memory/b_mmap.h>

#include <boost/algorithm/string/replace.hpp>

#include <string>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Memory::Containers;

Status::Code WebClientHdlr::action_download()
{
    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    std::string file = vars->getStringValue("file");
    bool asAttachment = (vars->getStringValue("dl") == "1");

    LocalRequestedFileInfo info;
    if (!resolveExistingPath(file, info) || info.isDirectory)
    {
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    auto mmap = std::make_shared<B_MMAP>();
    if (!mmap->referenceFile(info.fullPath, true, false))
    {
        // Fallback: serve as plain text error (binary file not readable).
        getResponseContentStreamableObject()->writeString(HTML_404);
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    serverResponse.setContentDataStreamer(mmap);
    detectContentTypeFromFilePath(info.fullPath);

    if (asAttachment)
    {
        std::string clean = ActionCommon::baseName(info.relativePath);
        // Sanitize against header injection.
        boost::replace_all(clean, "\"", "_");
        boost::replace_all(clean, "\r", "_");
        boost::replace_all(clean, "\n", "_");
        serverResponse.headers.replace("Content-Disposition", "attachment; filename=\"" + clean + "\"");
    }

    return respond(HTTP::Status::Code::S_200_OK, info.relativePath);
}