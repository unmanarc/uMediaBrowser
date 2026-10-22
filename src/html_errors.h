#pragma once

static constexpr const char *HTML_404 = R"(<center>404: Not Found.</center>)";

static constexpr const char *HTML_401 = R"(401: Unauthorized.)";

// 503 HTTP full response (headers + body). CRLF required by HTTP protocol.
static constexpr const char *HTML_503_RESPONSE =
        "HTTP/1.1 503 Service Temporarily Unavailable\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<center><h1>503 Service Temporarily Unavailable</h1></center>\r\n"
        "<hr>\r\n";

