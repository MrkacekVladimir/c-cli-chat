#include <WS2tcpip.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>

typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

DWORD WINAPI handle_connection(LPVOID lpThreadParameter) {
  printf("CHILD THREAD param %p\n", lpThreadParameter);
  SOCKET clientSock = *(SOCKET *)lpThreadParameter;
  printf("CHILD SOCKET %llu\n", clientSock);

  SocketAddrIn client;
  int clientSize = sizeof(client);
  getpeername(clientSock, (SocketAddr *)&client, &clientSize);

  char *address = inet_ntoa(client.sin_addr);
  printf("Client connected: %s\n", address);
  while (1) {
    char buffer[2048];
    int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
      printf("Recv failed: %d\n", WSAGetLastError());
      break;
    } else if (bytesReceived == 0) {
      printf("Connection closed by peer.\n");
      break;
    } else {
      buffer[bytesReceived] = '\0';
      printf("Received: %s\n", buffer);
    }
  }

  return 1;
}

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

  SocketAddrIn server;
  server.sin_family = AF_INET;
  server.sin_port = htons(8081);
  server.sin_addr.s_addr = htonl(0);

  int bindResult = bind(sockfd, (SocketAddr *)&server, sizeof(server));
  if (bindResult == SOCKET_ERROR) {
    printf("Could not bind a socket: %lu\n", GetLastError());
    goto sock_cleanup;
  }

  int listenResult = listen(sockfd, SOMAXCONN);
  if (listenResult == SOCKET_ERROR) {
    printf("Could not listen to a socket: %lu\n", GetLastError());
    goto sock_cleanup;
  }

  while (1) {
    SocketAddrIn client;
    int clientSize = sizeof(client);
    printf("Accepting connection...\n");
    SOCKET clientSock = accept(sockfd, (SocketAddr *)&client, &clientSize);
    if (clientSock == INVALID_SOCKET) {
      printf("Could not accept: %lu\n", GetLastError());
      goto sock_cleanup;
    }

    printf("MAIN THREAD param %p\n", &clientSock);
    HANDLE hThread =
        CreateThread(NULL, 0, handle_connection, (LPVOID)&clientSock, 0, NULL);
    if (hThread == NULL) {
      printf("CreateThread failed: %lu\n", GetLastError());
      closesocket(clientSock);
    } else {
      CloseHandle(hThread);
    }
  }

sock_cleanup:
  closesocket(sockfd);

wsa_cleanup:
  WSACleanup();

  return 1;
}
