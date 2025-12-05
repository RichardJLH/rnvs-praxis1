#include "packet.h"
#include <stdbool.h>
#include <string.h>

const char *generate_response(const bool packet_valid,
                              const packet request_packet) {
  static const char *const BAD_REQUEST =
      "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
  static const char *const NOT_FOUND =
      "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
  static const char *const OTHER =
      "HTTP/1.1 501 Not implemented\r\nContent-Length: 0\r\n\r\n";

  if (!packet_valid) {
    return BAD_REQUEST;
  }

  if (strcmp(request_packet.request_line.method, "GET") == 0) {
    return NOT_FOUND;
  }

  return OTHER;
}
