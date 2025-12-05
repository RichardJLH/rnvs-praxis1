#include "resources.h"
#include <stdbool.h>
#include <string.h>

Resources initial_static() {
  const Resource resource_array[] = {
      (Resource){
          .uri = "/static/foo",
          .data = "Foo",
      },
      (Resource){
          .uri = "/static/bar",
          .data = "Bar",
      },
      (Resource){
          .uri = "/static/baz",
          .data = "Baz",
      },
  };
  Resources resources;
  resources.count = sizeof resource_array / sizeof(Resource);

  for (size_t i = 0; i < resources.count; ++i) {
    const Resource resource = resource_array[i];
    strcpy(resources.resources[i].uri, resource.uri);
    strcpy(resources.resources[i].data, resource.data);
  }

  return resources;
}

const Resource *query_resource(const Resources *const resources,
                               const char *const uri) {
  for (size_t i = 0; i < resources->count; ++i) {
    const Resource *const resource = resources->resources + i;
    if (strcmp(resource->uri, uri) == 0) {
      return resource;
    }
  }
  return NULL;
}
const char *query_resource_data(const Resources *const resources,
                                const char *const uri) {
  const Resource *const resource = query_resource(resources, uri);
  return resource == NULL ? NULL : resource->data;
}
void append_resource(Resources *const resources, const char *const uri,
                     const char *const data) {
  Resource *const new = resources->resources + resources->count;
  strcpy(new->uri, uri);
  strcpy(new->data, data);
  ++resources->count;
}
bool write_resource(Resources *const resources, const char *const uri,
                    const char *const data) {
  char *const resource = (char *)query_resource_data(resources, uri);
  if (resource == NULL) {
    append_resource(resources, uri, data);
    return true;
  } else {
    strcpy(resource, data);
    return false;
  }
}
bool delete_resource(Resources *const resources, const char *const uri) {
  Resource *const resource = (Resource *)query_resource(resources, uri);
  if (resource == NULL) {
    return false;
  }

  const size_t index = resource - resources->resources;
  memmove(resource, resource + 1,
          (resources->count - index) * sizeof(Resource));
  --resources->count;
  return true;
}

bool is_modifiable(const char *const uri) {
  static const char *const PREFIX = "/dynamic/";
  return strncmp(uri, PREFIX, strlen(PREFIX)) == 0;
}
