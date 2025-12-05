#include "respond.h"
#include "../resources/resources.h"
#include "packet.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void generate_get_response(Response *const response, const char *const uri,
                           const Resources *const resources) {
  const char *const data = query_resource(uri, resources);
  if (data != NULL) {
    response->code = "200";
    response->explanation = "Ok";
    strcpy(response->body, data);
  } else {
    response->code = "404";
    response->explanation = "Not Found";
    strcpy(response->body, "");
  }
}

void generate_response(Response *const response, const bool packet_valid,
                       const packet request_packet,
                       const Resources *const resources) {
  if (!packet_valid) {
    response->code = "400";
    response->explanation = "Bad Request";
    strcpy(response->body, "");
    return;
  }

  if (strcmp(request_packet.request_line.method, "GET") == 0) {
    generate_get_response(response, request_packet.request_line.uri, resources);
  } else {
    response->code = "501";
    response->explanation = "Not implemented";
    strcpy(response->body, "");
  }
}
void format_response(char *const string, const Response *const response) {
  snprintf(string, MAX_RESPONSE_SIZE,
           "HTTP/1.1 %s %s\r\nContent-Length: %zu\r\n\r\n%s", response->code,
           response->explanation, strlen(response->body), response->body);
}
