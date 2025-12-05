#ifndef HTTP_PACKET_H
#define HTTP_PACKET_H

#define MAX_REQUEST_LINE_SIZE 256
#define MAX_PACKET_SIZE 8192
#define MAX_HEADER_SIZE 256
#define MAX_HEADER_NUMBER 40

#include <sys/types.h>

typedef struct {
  char method[MAX_REQUEST_LINE_SIZE];
  char uri[MAX_REQUEST_LINE_SIZE];
  char version[MAX_REQUEST_LINE_SIZE];
} start_line;

typedef struct {
  char key[MAX_HEADER_SIZE];
  char value[MAX_HEADER_SIZE];
} header;

typedef struct {
  header content[MAX_HEADER_NUMBER];
  size_t number;
} headers;

typedef struct {
  start_line request_line;
  headers headers_section;
  char body[MAX_PACKET_SIZE];
} packet;

#endif
