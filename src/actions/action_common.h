#pragma once

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace ActionCommon
{
    // Split a comma separated list (ignoring empty tokens).
    inline std::vector<std::string> splitComma(const std::string &s)
    {
        std::vector<std::string> out;
        boost::split(out, s, boost::is_any_of(","), boost::token_compress_on);
        return out;
    }

    // Reserved control files that the server uses internally.
    inline bool isReservedDotFile(const std::string &name)
    {
        return name == ".dontupload" || name == ".uploadname" || name == ".upload";
    }

    // Extract the base name from a URI-style path ("/a/b.txt" -> "b.txt").
    inline std::string baseName(const std::string &path)
    {
        return std::filesystem::path(path).filename().string();
    }
}