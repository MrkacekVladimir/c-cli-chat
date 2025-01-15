#include <WS2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

int main() {
  WSADATA wsaData;
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (result != 0) {
    printf("WSAStartup failed: %d\n", result);
    return 1;
  }

  SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == INVALID_SOCKET) {
    printf("Could not create a socket: %lu\n", GetLastError());
    goto wsa_cleanup;
  }

  SocketAddrIn serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8081);
  inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

  int connRes = connect(sockfd, (SocketAddr *)&serverAddr, sizeof(serverAddr));
  if (connRes == SOCKET_ERROR) {
    printf("Could not create a socket: %lu\n", GetLastError());
    goto sock_cleanup;
  }

  printf("Connected to the server");

  while (1) {
    char buffer[2048];
    puts("Zpráva: ");
    gets_s(buffer, sizeof(buffer));

    int sendRes = send(sockfd, buffer, strnlen(buffer, sizeof(buffer)), 0);
    if (sendRes == SOCKET_ERROR) {
      printf("Send failed: %d\n", WSAGetLastError());
      goto sock_cleanup;
    }
  }

sock_cleanup:
  closesocket(sockfd);

wsa_cleanup:
  WSACleanup();

  return 1;
}
