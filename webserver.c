#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 10

void fail(const char *const message) {
  perror(message);
  exit(EXIT_FAILURE);
}

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
ssize_t find_http_packet_length(const char *const buffer) {
  const char *const terminator = "\r\n\r\n";
  const char *end = strstr(buffer, terminator);

  return (end == NULL) ? -1 : (ssize_t)(end - buffer + strlen(terminator));
}

void handle_client(const int client_socket) {
  const char *reply = "Reply\r\n\r\n";

  char received[512];
  size_t received_end = 0;
  ssize_t received_bytes = recv(client_socket, received + received_end,
                                sizeof(received) - received_end, 0);
  while (received_bytes > 0) {
    received_end += received_bytes;
    received[received_end] = '\0';

    ssize_t next_packet_length = find_http_packet_length(received);

    while (next_packet_length != -1) {
      send_message(client_socket, reply);

      memmove(received, received + next_packet_length,
              received_end - next_packet_length);
      received_end -= next_packet_length;
      received[received_end] = '\0';

      next_packet_length = find_http_packet_length(received);
    }

    received_bytes = recv(client_socket, received + received_end,
                          sizeof(received) - received_end, 0);
  }

  if (received_bytes == -1) {
    perror("Receiving bytes");
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fputs("Error parsing program arguments: Needs exactly two arguments\n",
          stderr);
    exit(EXIT_FAILURE);
  }

  struct addrinfo *const address_info = get_address_info(argv[1], argv[2]);
  const int listener_socket = setup_socket(address_info);
  bind_socket(listener_socket, address_info);
  freeaddrinfo(address_info);

  listen_socket(listener_socket);

  while (true) {
    const int client_socket = accept_client(listener_socket);
    handle_client(client_socket);
    close(client_socket);
  }

  return 0;
}
