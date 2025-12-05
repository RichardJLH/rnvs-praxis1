#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>

struct addrinfo *get_address_info(const char *const, const char *const);
int setup_socket(const struct addrinfo *const);
void bind_socket(const int, const struct addrinfo *const);
void listen_socket(const int);

int accept_client(const int);

void send_message(const int, const char *const);
ssize_t receive(const int, char *const, const size_t);

#endif
