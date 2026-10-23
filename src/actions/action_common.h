#pragma once

#include <string>
#include <vector>

namespace ActionCommon
{
    // Split a comma separated list (ignoring empty tokens).
    inline std::vector<std::string> splitComma(const std::string &s)
    {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s)
        {
            if (c == ',')
            {
                if (!cur.empty())
                    out.push_back(cur);
                cur.clear();
            }
            else
            {
                cur += c;
            }
        }
        if (!cur.empty())
            out.push_back(cur);
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
        size_t p = path.find_last_of('/');
        if (p == std::string::npos)
            return path;
        std::string b = path.substr(p + 1);
        if (b.empty())
            return path;
        return b;
    }
}