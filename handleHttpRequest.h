#ifndef HANDLEHTTPREQUEST_H
#define HANDLEHTTPREQUEST_H

#include <string>

namespace HTTP
{
  static const std::string indexHtml = "<b>MAIN PAGE</b>";

  static const std::string response404 = "HTTP/1.0 404 NOT FOUND\r\n"
                                          "Content-length: 0\r\n"
                                          "Content-Type: text/html\r\n\r\n";

  static const std::string response200 = "HTTP/1.0 200 OK\r\n"
                                          "Content-length: " + std::to_string(indexHtml.length()) + "\r\n"
                                          "Connection: close\r\n"
                                          "Content-Type: text/html\r\n"
                                          "\r\n";
}

#endif
