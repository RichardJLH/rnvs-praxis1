#include "http/parse.h"
#include "http/respond.h"
#include "network/network.h"
#include "util/failure.h"
#include "util/string_view.h"
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void handle_client(const int client_socket) {
  char buffer[MAX_PACKET_SIZE];
  char *data_end = buffer;
  const char *const buffer_end = buffer + sizeof buffer;

  ssize_t received_bytes;
  while ((received_bytes =
              receive(client_socket, data_end, buffer_end - data_end)) > 0) {
    data_end += received_bytes;
    *data_end = '\0';

    ssize_t packet_length;
    while ((packet_length =
                find_http_packet_length(buffer, data_end - buffer)) != -1) {
      const string_view packet_raw =
          (string_view){.begin = buffer, .end = buffer + packet_length};
      packet http_packet;
      const bool packet_valid = parse_packet(packet_raw, &http_packet);
      const char *const response = generate_response(packet_valid, http_packet);
      send_message(client_socket, response);

      memmove(buffer, buffer + packet_length,
              (data_end - buffer) - packet_length);
      data_end -= packet_length;
      *data_end = '\0';
    }
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
