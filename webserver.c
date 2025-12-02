#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

  const int optval = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

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

  const char *reply = "Reply\r\n\r\n";
  const unsigned long reply_size = strlen(reply) * sizeof(char);

  while (true) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    const int s1 = accept(s, (struct sockaddr *)&their_addr, &addr_size);
    if (s1 == -1) {
      fprintf(stderr, "Error accepting connection");
      exit(EXIT_FAILURE);
    }

    char received[512];
    size_t received_end = 0;
    while (true) {
      const ssize_t received_bytes =
          recv(s1, received + received_end, sizeof(received) - received_end, 0);
      if (received_bytes == -1) {
        perror("Receiving bytes");
        break;
      }
      if (received_bytes == 0) {
        break;
      }

      received_end += received_bytes;

      while (true) {
        received[received_end] = '\0';
        const char *const terminator = "\r\n\r\n";
        const char *end = strstr(received, terminator);
        if (end == NULL)
          break;

        end += strlen(terminator);

        send(s1, reply, reply_size, 0);

        const size_t packet_len = end - received;
        memmove(received, received + packet_len, received_end - packet_len);
        received_end -= packet_len;
      }
    }
    close(s1);
  }

  return 0;
}
