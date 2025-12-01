#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int n, char **arg) {
  if (n != 3)
    return -1;

  struct addrinfo hints = {
      .ai_family = AF_INET,
  };
  struct addrinfo *res;

  int errorcode = getaddrinfo(arg[1], arg[2], &hints, &res);
  if (errorcode) {
    fprintf(stderr, "Error parsing host/port");
    exit(EXIT_FAILURE);
  }

  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  errorcode = bind(s, res->ai_addr, res->ai_addrlen);
  if (errorcode == -1) {
    fprintf(stderr, "Error binding to socket");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(res);

  errorcode = listen(s, 10);
  if (errorcode == -1) {
    fprintf(stderr, "Error listening to socket");
    exit(EXIT_FAILURE);
  }

  return 0;
}
