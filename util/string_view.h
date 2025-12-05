#ifndef UTIL_STRING_VIEW_H
#define UTIL_STRING_VIEW_H

typedef struct {
  const char *begin;
  const char *end;
} string_view;

const char *find_header_separator(const string_view);
const char *find_line_break(const string_view);
const char *find_empty_line(const string_view);

#endif
