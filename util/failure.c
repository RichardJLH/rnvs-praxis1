#include "stdio.h"
#include "stdlib.h"

void fail(const char *const message) {
  perror(message);
  exit(EXIT_FAILURE);
}
