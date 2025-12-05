#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

#include "../util/string_view.h"
#include "packet.h"
#include <stdbool.h>
#include <sys/types.h>

ssize_t find_http_packet_length(const char *, size_t);
bool parse_request(string_view, start_line *);
bool parse_header(string_view, header *);
bool parse_headers(string_view, headers *);
bool parse_packet(string_view, packet *);

#endif
