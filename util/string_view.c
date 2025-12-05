#include "string_view.h"
#include "stddef.h"

const char *find_header_separator(const string_view string) {
  for (const char *it = string.begin; it + 1 < string.end; ++it) {
    if (it[0] == ':' && it[1] == ' ') {
      return it;
    }
  }
  return NULL;
}
const char *find_line_break(const string_view string) {
  for (const char *it = string.begin; it + 1 < string.end; ++it) {
    if (it[0] == '\r' && it[1] == '\n') {
      return it;
    }
  }
  return NULL;
}
const char *find_empty_line(const string_view string) {
  for (const char *it = string.begin; it + 3 < string.end; ++it) {
    if (it[0] == '\r' && it[1] == '\n' && it[2] == '\r' && it[3] == '\n') {
      return it;
    }
  }
  return NULL;
}
