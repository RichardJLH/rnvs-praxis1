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

const char *query_resource(const char *const uri,
                           const Resources *const resources) {
  for (size_t i = 0; i < resources->count; ++i) {
    const Resource *const resource = resources->resources + i;
    if (strcmp(resource->uri, uri) == 0) {
      return resource->data;
    }
  }
  return NULL;
}
