#include "../util/string_view.h"
#include "packet.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

ssize_t find_http_packet_length(const char *const buffer, size_t used) {
  const char *const header_end = strstr(buffer, "\r\n\r\n");
  if (header_end == NULL) {
    return -1;
  }

  size_t header_length = header_end - buffer + 4;

  int content_length = 0;
  const char *p = buffer;
  while (p < header_end) {
    static const char *const KEY = "Content-Length:";
    const size_t key_length = strlen(KEY);

    if (strncasecmp(p, KEY, key_length) == 0) {
      content_length = atoi(p + key_length);
      break;
    }
    const char *const line_break = strstr(p, "\r\n");
    if (line_break == NULL) {
      break;
    }
    p = line_break + 2;
  }

  const size_t total = header_length + content_length;
  return (used < total) ? -1 : (ssize_t)total;
}

bool parse_request(const string_view raw, start_line *const result) {
  const char *const method_end = memchr(raw.begin, ' ', raw.end - raw.begin);
  if (method_end == NULL) {
    return false;
  }

  const char *const rest_after_method = method_end + 1;
  const char *const uri_end =
      memchr(rest_after_method, ' ', raw.end - rest_after_method);
  if (uri_end == NULL) {
    return false;
  }
  const char *const rest_after_uri = uri_end + 1;
  if (rest_after_uri == raw.end) {
    return false;
  }
  const char *const version_end = raw.end;

  const char *const method_begin = raw.begin;
  const char *const uri_begin = rest_after_method;
  const char *const version_begin = rest_after_uri;

  const size_t method_length = method_end - method_begin;
  const size_t uri_length = uri_end - uri_begin;
  const size_t version_length = version_end - version_begin;

  memcpy(result->method, raw.begin, method_length);
  memcpy(result->uri, uri_begin, uri_length);
  memcpy(result->version, version_begin, version_length);

  result->method[method_length] = '\0';
  result->uri[uri_length] = '\0';
  result->version[version_length] = '\0';

  return true;
}

bool parse_header(const string_view raw, header *const result) {
  const char *const separator = find_header_separator(raw);
  if (separator == NULL) {
    return false;
  }

  const char *const key_begin = raw.begin;
  const size_t key_length = separator - key_begin;
  memcpy(result->key, key_begin, key_length);
  result->key[key_length] = '\0';

  const char *const value_begin = separator + 2;
  const size_t value_length = raw.end - value_begin;
  memcpy(result->value, value_begin, value_length);
  result->value[value_length] = '\0';

  return true;
}

bool parse_headers(const string_view raw, headers *const result) {
  result->number = 0;

  const char *line_begin = raw.begin;
  const char *line_end;
  while ((line_end = find_line_break(
              (string_view){.begin = line_begin, .end = raw.end})) != NULL) {
    const string_view line = (string_view){
        .begin = line_begin,
        .end = line_end,
    };
    const bool valid_header =
        parse_header(line, result->content + result->number);
    if (!valid_header) {
      return false;
    }
    ++result->number;

    line_begin = line_end + 2;
  }
  return true;
}

bool parse_packet(const string_view raw, packet *const result) {
  const string_view start_line_raw = (string_view){
      .begin = raw.begin,
      .end = find_line_break(raw),
  };
  if (start_line_raw.end == NULL) {
    return false;
  }

  start_line request_line;
  const bool valid_request = parse_request(start_line_raw, &request_line);
  if (!valid_request) {
    return false;
  }

  const char *const empty_line = find_empty_line(
      (string_view){.begin = start_line_raw.end, .end = raw.end});
  if (empty_line == NULL) {
    return false;
  }
  const string_view headers_raw = (string_view){
      .begin = start_line_raw.end + 2,
      .end = empty_line + 2,
  };
  headers headers_section;
  const bool valid_headers = parse_headers(headers_raw, &headers_section);
  if (!valid_headers) {
    return false;
  }

  const string_view body_raw = (string_view){
      .begin = headers_raw.end + 2,
      .end = raw.end,
  };

  result->request_line = request_line;
  result->headers_section = headers_section;

  memcpy(result->body, body_raw.begin, body_raw.end - body_raw.begin);
  result->body[body_raw.end - body_raw.begin] = '\0';

  return true;
}
