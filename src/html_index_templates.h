#pragma once

#include <inttypes.h>

// Index page header. Used with strPrintf:
//   %s = path (title), %s = CSS block, %s = JS block, %s = path (h1), %s = targz link, %s = upload form
static constexpr const char *HTML_INDEX_HEADER =
        "<html>\n"
        "<head>\n"
        "    <title>Index of %s</title>\n"
        "%s\n"
        "%s\n"
        "</head>\n"
        "<body>\n"
        "        <table align='center' width='80%%' class='tableupload'>\n"
        "          <tr class='tableupload'><td class='tableupload'><h1>Index of %s</h1></td></tr>\n"
        "          %s\n"
        "        </table>\n"
        "    %s\n"
        "    <hr><center><br><br>\n"
        "    <table align='center' width='80%%'>\n"
        "        <tr bgcolor='#0000B0'>\n"
        "            <td><b>FileName</b></td>\n"
        "            <td><b>Size</b></td>\n"
        "            <td><b>Modification Time</b></td>\n"
        "            <td><b>UID:GID</b></td>\n"
        "            <td width=20px><b>Permissions</b></td>\n"
        "        </tr>\n";

// Table row for one file/directory entry. Used with strPrintf:
//   %s = folder/file icon prefix, %s = URL-encoded href, %s = HTML-encoded name,
//   %s = icon suffix, %s = size, %s = modification time,
//   %" PRIi32 ":%" PRIi32 " = UID:GID, %s = permissions
static constexpr const char *HTML_INDEX_ROW =
        "        <tr>\n"
        "            <td>%s<a href='%s'>%s</a>%s</td>\n"
        "            <td>%s</td>\n"
        "            <td>%s</td>\n"
        "            <td>%" PRIi32 ":%" PRIi32 "</td>\n"
        "            <td>%s</td>\n"
        "        </tr>\n";

static constexpr const char *HTML_INDEX_TABLE_CLOSE = "    </table>\n";

// Folder not accessible error
static constexpr const char *HTML_INDEX_FOLDER_INACCESSIBLE =
        "    </table>\n"
        "<h2>Folder not accessible</h2>";

// Index page footer. Used with strPrintf: %s = software version
static constexpr const char *HTML_INDEX_FOOTER =
        "    </center><br><br><br><hr>\n"
        "    <center>\n"
        "        Provided by <a href='https://github.com/unmanarc/uSimpleWebServer3'>Unmanarc HTTP Simple Web Server v%s</a>\n"
        "    </center>\n"
        "</body>\n"
        "</html>\n";

// Inline HTML snippets used with snprintf for permission display
static constexpr const char *HTML_CHMOD_WARNING = "<font color=red>%o*</font>\n";
static constexpr const char *HTML_CHMOD_NORMAL = "%o\n";

// Icon HTML prefixes/suffixes for table rows
static constexpr const char *HTML_ICON_FOLDER_PREFIX = "<div class=folder16>[";
static constexpr const char *HTML_ICON_FOLDER_SUFFIX = "]</div>";
static constexpr const char *HTML_ICON_FILE_PREFIX = "<div class=file16>";
static constexpr const char *HTML_ICON_FILE_SUFFIX = "</div>";

