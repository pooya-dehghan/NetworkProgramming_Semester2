#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

int main()
{

#if defined(_WIN32)
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d))
  {
    fprintf(stderr, "Failed to initialize.\n");
    return 1;
  }
#endif
  printf("Configuring local address for our beloved server.\n");
  struct addrinfo hints;
  memset(&hints, 0 , sizeof(hints));
  hints.ai_family = AF_INET; // for telling our program which uses the IPv4 protocol not IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *bind_address;
  getaddrinfo(0, "8080", &hints ,&bind_address);
  printf("Creating our beautiful Socket...\n");
  SOCKET sock_listen;
  sock_listen = socket(bind_address -> ai_family , bind_address -> ai_socktype ,bind_address -> ai_protocol);
  // we wanna make sure everyting is ok up to this point so we call ISVALIDSOCKET to check if there is any error
  if(!ISVALIDSOCKET(sock_listen)){
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  // now the time is to bind our socket to our address
  if(bind(sock_listen , bind_address -> ai_addr, bind_address ->ai_addrlen)){
    fprintf(stderr ,"bind() failed . (%d)\n" , GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address);
  // now we listen for new connections
  printf("Listening...\n");
  if(listen(sock_listen , 10) < 0){
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  // now is the time to use select 
  fd_set master;
  FD_ZERO(&master);
  FD_SET(sock_listen ,  &master);
  SOCKET max_socket = sock_listen;
  printf("Waiting for connections.\n");
  while(1){
    fd_set reads;
    reads = master;
    if(select(max_socket + 1, &reads, 0,0,0) < 0){
      fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
      return 1;
    }
    SOCKET i;
    for(i = 1; i<= max_socket ; i++){
      if(FD_ISSET(i,&reads)){
        //handle socket
    if(i == sock_listen){
      struct sockaddr_storage client_address;
      socklen_t client_len = sizeof(client_address);
      SOCKET socket_client = accept(sock_listen, (struct sockaddr*) &client_address,&client_len);
      if (!ISVALIDSOCKET(socket_client)) {
        fprintf(stderr, "accept() failed. (%d)\n",
                GETSOCKETERRNO());
        return 1;
        }
        FD_SET(sock_listen , &master);
        if(socket_client > max_socket)
          max_socket = socket_client;
        char address_buffer[100];
        getnameinfo((struct sockaddr *)&client_address,
          client_len,
          address_buffer, sizeof(address_buffer), 0, 0,
          NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);
    } else {
          char read[1024];
          int bytes_received = recv(i, read, 1024, 0);
          if (bytes_received < 1) {
              FD_CLR(i, &master);
              CLOSESOCKET(i);
              continue;
          }

          int j;
          for (j = 0; j < bytes_received; ++j)
              read[j] = toupper(read[j]);
          send(i, read, bytes_received, 0);
        }
      }
    }
  }
  
  printf("Closing listening socket...\n");
  CLOSESOCKET(sock_listen);

#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");


  return 0;
}