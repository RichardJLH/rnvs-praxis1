#ifndef HTTP_RESPOND
#define HTTP_RESPOND

#include "../util/string_view.h"
#include "packet.h"
#include <stdbool.h>

const char *generate_response(bool, packet);

#endif
