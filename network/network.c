#include "../util/failure.h"
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 10

struct addrinfo *get_address_info(const char *const ip,
                                  const char *const port) {
  struct addrinfo *res;
  struct addrinfo hints = {
      .ai_family = AF_INET,
  };

  const int error_code = getaddrinfo(ip, port, &hints, &res);
  if (error_code != 0) {
    fail("Error parsing host/port");
  }

  return res;
}
int setup_socket(const struct addrinfo *const address_info) {
  const int s = socket(address_info->ai_family, address_info->ai_socktype,
                       address_info->ai_protocol);

  const int optval = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  return s;
}
void bind_socket(const int socket, const struct addrinfo *const address_info) {
  const int error_code =
      bind(socket, address_info->ai_addr, address_info->ai_addrlen);
  if (error_code != 0) {
    fail("Error binding to socket");
  }
}
void listen_socket(const int socket) {
  const int error_code = listen(socket, BACKLOG);
  if (error_code == -1) {
    perror("Error listening to socket");
  }
}

int accept_client(const int listener_socket) {
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof(their_addr);

  const int client_socket =
      accept(listener_socket, (struct sockaddr *)&their_addr, &addr_size);
  if (client_socket == -1) {
    fail("Error accepting connection");
  }
  return client_socket;
}

void send_message(const int socket, const char *const message) {
  const size_t message_size = strlen(message);

  ssize_t sent;
  for (size_t total_sent = 0; total_sent < message_size; total_sent += sent) {
    sent = send(socket, message + total_sent, message_size - total_sent, 0);
    if (sent == -1) {
      close(socket);
      fail("Error sending reply");
    }
  }
}
ssize_t receive(const int socket, char *const buffer,
                const size_t buffer_capacity) {
  return recv(socket, buffer, buffer_capacity, 0);
}
