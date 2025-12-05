#include "respond.h"
#include "../resources/resources.h"
#include "packet.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void generate_get_response(Response *const response, const char *const uri,
                           const Resources *const resources) {
  const char *const data = query_resource_data(resources, uri);
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
void generate_put_response(Response *const response, Resources *const resources,
                           const char *const uri, const char *const data) {
  if (!is_modifiable(uri)) {
    response->code = "403";
    response->explanation = "Forbidden";
    strcpy(response->body, "");
    return;
  }

  const bool created = write_resource(resources, uri, data);
  if (created) {
    response->code = "201";
    response->explanation = "Created";
    strcpy(response->body, "");
  } else {
    response->code = "204";
    response->explanation = "No Content";
    strcpy(response->body, "");
  }
}
void generate_delete_response(Response *const response,
                              Resources *const resources,
                              const char *const uri) {
  if (!is_modifiable(uri)) {
    response->code = "403";
    response->explanation = "Forbidden";
    strcpy(response->body, "");
    return;
  }

  const bool deleted = delete_resource(resources, uri);
  if (deleted) {
    response->code = "204";
    response->explanation = "No Content";
    strcpy(response->body, "");
  } else {
    response->code = "404";
    response->explanation = "Not Found";
    strcpy(response->body, "");
  }
}

void generate_response(Response *const response, Resources *const resources,
                       const bool packet_valid, const packet request_packet) {
  if (!packet_valid) {
    response->code = "400";
    response->explanation = "Bad Request";
    strcpy(response->body, "");
    return;
  }

  if (strcmp(request_packet.request_line.method, "GET") == 0) {
    generate_get_response(response, request_packet.request_line.uri, resources);
  } else if (strcmp(request_packet.request_line.method, "PUT") == 0) {
    generate_put_response(response, resources, request_packet.request_line.uri,
                          request_packet.body);
  } else if (strcmp(request_packet.request_line.method, "DELETE") == 0) {
    generate_delete_response(response, resources,
                             request_packet.request_line.uri);
  } else {
    response->code = "501";
    response->explanation = "Not Implemented";
    strcpy(response->body, "");
  }
}
void format_response(char *const string, const Response *const response) {
  snprintf(string, MAX_RESPONSE_SIZE,
           "HTTP/1.1 %s %s\r\nContent-Length: %zu\r\n\r\n%s", response->code,
           response->explanation, strlen(response->body), response->body);
}
