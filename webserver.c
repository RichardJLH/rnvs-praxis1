#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 10
#define MAX_REQUEST_LINE_SIZE 256
#define MAX_PACKET_SIZE 8192
#define MAX_HEADER_SIZE 256
#define MAX_HEADER_NUMBER 40

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
ssize_t receive(const int socket, char *const buffer,
                const size_t buffer_capacity) {
  return recv(socket, buffer, buffer_capacity, 0);
}
ssize_t find_http_packet_length(const char *const buffer, size_t used) {
  const char *const header_end = strstr(buffer, "\r\n\r\n");
  if (header_end == NULL) {
    return -1;
  }

  size_t header_length = header_end - buffer + 4;

  int content_length = 0;
  const char *p = buffer;
  while (p < header_end) {
    static const char *const KEY = "Content-Length:";
    const size_t key_length = strlen(KEY);

    if (strncasecmp(p, KEY, key_length) == 0) {
      content_length = atoi(p + key_length);
      break;
    }
    const char *const line_break = strstr(p, "\r\n");
    if (line_break == NULL) {
      break;
    }
    p = line_break + 2;
  }

  const size_t total = header_length + content_length;
  return (used < total) ? -1 : (ssize_t)total;
}

typedef struct {
  const char *begin;
  const char *end;
} bounded_string;

typedef struct {
  char method[MAX_REQUEST_LINE_SIZE];
  char uri[MAX_REQUEST_LINE_SIZE];
  char version[MAX_REQUEST_LINE_SIZE];
} start_line;

bool parse_request(const bounded_string raw, start_line *const result) {
  const char *const method_end = memchr(raw.begin, ' ', raw.end - raw.begin);
  if (method_end == NULL) {
    return false;
  }

  const char *const rest_after_method = method_end + 1;
  const char *const uri_end =
      memchr(rest_after_method, ' ', raw.end - rest_after_method);
  if (uri_end == NULL) {
    return false;
  }
  const char *const rest_after_uri = uri_end + 1;
  if (rest_after_uri == raw.end) {
    return false;
  }
  const char *const version_end = raw.end;

  const char *const method_begin = raw.begin;
  const char *const uri_begin = rest_after_method;
  const char *const version_begin = rest_after_uri;

  const size_t method_length = method_end - method_begin;
  const size_t uri_length = uri_end - uri_begin;
  const size_t version_length = version_end - version_begin;

  memcpy(result->method, raw.begin, method_length);
  memcpy(result->uri, uri_begin, uri_length);
  memcpy(result->version, version_begin, version_length);

  result->method[method_length] = '\0';
  result->uri[uri_length] = '\0';
  result->version[version_length] = '\0';

  return true;
}

const char *find_header_separator(const bounded_string string) {
  for (const char *it = string.begin; it + 1 < string.end; ++it) {
    if (it[0] == ':' && it[1] == ' ') {
      return it;
    }
  }
  return NULL;
}
const char *find_line_break(const bounded_string string) {
  for (const char *it = string.begin; it + 1 < string.end; ++it) {
    if (it[0] == '\r' && it[1] == '\n') {
      return it;
    }
  }
  return NULL;
}
const char *find_empty_line(const bounded_string string) {
  for (const char *it = string.begin; it + 3 < string.end; ++it) {
    if (it[0] == '\r' && it[1] == '\n' && it[2] == '\r' && it[3] == '\n') {
      return it;
    }
  }
  return NULL;
}

typedef struct {
  char key[MAX_HEADER_SIZE];
  char value[MAX_HEADER_SIZE];
} header;

bool parse_header(const bounded_string raw, header *const result) {
  const char *const separator = find_header_separator(raw);
  if (separator == NULL) {
    return false;
  }

  const char *const key_begin = raw.begin;
  const size_t key_length = separator - key_begin;
  memcpy(result->key, key_begin, key_length);
  result->key[key_length] = '\0';

  const char *const value_begin = separator + 2;
  const size_t value_length = raw.end - value_begin;
  memcpy(result->value, value_begin, value_length);
  result->value[value_length] = '\0';

  return true;
}

typedef struct {
  header content[MAX_HEADER_NUMBER];
  size_t number;
} headers;

// parses string http header into header struct
// if `raw` is a valid header, writes into `result` and returns `true`
// returns false otherwise
bool parse_headers(const bounded_string raw, headers *const result) {
  result->number = 0;

  const char *line_begin = raw.begin;
  const char *line_end;
  while ((line_end = find_line_break(
              (bounded_string){.begin = line_begin, .end = raw.end})) != NULL) {
    const bounded_string line = (bounded_string){
        .begin = line_begin,
        .end = line_end,
    };
    const bool valid_header =
        parse_header(line, result->content + result->number);
    if (!valid_header) {
      return false;
    }
    ++result->number;

    line_begin = line_end + 2;
  }
  return true;
}

typedef struct {
  start_line request_line;
  headers headers_section;
  char body[MAX_PACKET_SIZE];
} packet;

bool parse_packet(const bounded_string raw, packet *const result) {
  const bounded_string start_line_raw = (bounded_string){
      .begin = raw.begin,
      .end = find_line_break(raw),
  };
  if (start_line_raw.end == NULL) {
    return false;
  }

  start_line request_line;
  const bool valid_request = parse_request(start_line_raw, &request_line);
  if (!valid_request) {
    return false;
  }

  const char *const empty_line = find_empty_line(
      (bounded_string){.begin = start_line_raw.end, .end = raw.end});
  if (empty_line == NULL) {
    return false;
  }
  const bounded_string headers_raw = (bounded_string){
      .begin = start_line_raw.end + 2,
      .end = empty_line + 2,
  };
  headers headers_section;
  const bool valid_headers = parse_headers(headers_raw, &headers_section);
  if (!valid_headers) {
    return false;
  }

  const bounded_string body_raw = (bounded_string){
      .begin = headers_raw.end + 4,
      .end = raw.end,
  };

  result->request_line = request_line;
  result->headers_section = headers_section;

  memcpy(result->body, body_raw.begin, body_raw.end - body_raw.begin);
  result->body[body_raw.end - body_raw.begin] = '\0';

  return true;
}

const char *generate_response(const bounded_string request_packet_raw) {
  static const char *const BAD_REQUEST =
      "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
  static const char *const NOT_FOUND =
      "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
  static const char *const OTHER =
      "HTTP/1.1 501 Not implemented\r\nContent-Length: 0\r\n\r\n";

  packet http_packet;
  const bool packet_valid = parse_packet(request_packet_raw, &http_packet);
  if (!packet_valid) {
    return BAD_REQUEST;
  }

  if (strcmp(http_packet.request_line.method, "GET") == 0) {
    return NOT_FOUND;
  }

  return OTHER;
}

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
      const bounded_string packet_raw =
          (bounded_string){.begin = buffer, .end = buffer + packet_length};
      const char *const response = generate_response(packet_raw);
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
