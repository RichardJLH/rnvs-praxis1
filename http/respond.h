#ifndef HTTP_RESPOND
#define HTTP_RESPOND

#include "../resources/resources.h"
#include "../util/string_view.h"
#include "packet.h"
#include <stdbool.h>

#define MAX_RESPONSE_SIZE 8192

typedef struct {
  const char *code;
  const char *explanation;
  char body[MAX_RESPONSE_SIZE];
} Response;

void generate_response(Response *, Resources *, bool, packet);
void format_response(char *, const Response *);

#endif
