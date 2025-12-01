#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

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

 // while (1) {
  //}

struct sockaddr_storage their_addr;
socklen_t addr_size;

char recieved[512];
const char* sent = "Reply";

//while (1){
int s1 = accept(s, (struct sockaddr *)&their_addr, &addr_size);
recv(s1, recieved, 512, 0);
send(s1, sent, strlen("Reply"), 0); 
//}


  return 0;
}
