#ifndef RESOURCES_H
#define RESOURCES_H

#include <linux/limits.h>
#include <stdbool.h>
#include <sys/types.h>

#define MAX_RESOURCE_NUMBER 100
#define MAX_RESPONSE_SIZE 8192

typedef struct {
  char uri[PATH_MAX];
  char data[MAX_RESPONSE_SIZE];
} Resource;

typedef struct {
  Resource resources[MAX_RESOURCE_NUMBER];
  size_t count;
} Resources;

Resources initial_static();

const char *query_resource(const char *, const Resources *);

#endif
