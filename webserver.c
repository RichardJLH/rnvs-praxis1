#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/**
2 * Derives a sockaddr_in structure from the provided host and port information.
3 *
4 * @param host The host (IP address or hostname) to be resolved into a network
address.
5 * @param port The port number to be converted into network byte order.
6 *
7 * @return A sockaddr_in structure representing the network address derived from
the host and port.
8 */
static struct sockaddr_in derive_sockaddr(const char* host, const char* port) {
struct addrinfo hints = {
.ai_family = AF_INET,
};
struct addrinfo *result_info;

// Resolve the host (IP address or hostname) into a list of possible addresses.
int returncode = getaddrinfo(host, port, &hints, &result_info);
if (returncode) {
fprintf(stderr, "Error␣parsing␣host/port");
exit(EXIT_FAILURE);
}

// Copy the sockaddr_in structure from the first address in the list
struct sockaddr_in result = *((struct sockaddr_in*) result_info->ai_addr);

// Free the allocated memory for the result_info
freeaddrinfo(result_info);
return result;
}


int 
main(int n, char** arg)
{
if(n != 3) return -1;
struct sockaddr_in res = derive_sockaddr(arg[1], arg[2]);
int s = socket(res.ai_family, res.ai_socktype, res.ai_protocol);
bind(sockfd, res.ai_addr, res.ai_addrlen);
listen(s, 1);
}

