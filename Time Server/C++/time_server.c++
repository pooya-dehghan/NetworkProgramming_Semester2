#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

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

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <string>
using namespace std;

int main() {
  // first of all we need to initial the socket;
#if defined(_WIN32)
  WSADATA d;
  if(WSAStartup(MAKEWORD(2,2) , &d)){
    cout<<"Failed to continue"<<stderr;
  }
#endif
  // now we need to configure the socket;
  cout<<"configuring local address and socket..."<<endl;
  // we need to define a data type called addrinfo
  // this addrinfo is gonna hold up the configurations of our server
  struct addrinfo hints;
  // addrinfo *hintsToNull = &hints;
  // hintsToNull = NULL;
  memset(&hints,0,sizeof(hints));
  // now we are gonna reach out to data members of this structure
  hints.ai_family = AF_INET; // specifies we are looking for IPv4 address
  hints.ai_socktype = SOCK_STREAM; // specifies that we are gonna need tcp protocol
  hints.ai_flags = AI_PASSIVE; // we are gonna need wildcard address
  // now we define a pointer to the object in out program
  // to be able to have an address and name of server available
  struct addrinfo *bind_address;
  // here now we use getaddrinfo function which return a addrinfo structure that have internet address
  // which can be used in bind() and connect() 
  string localhostNumber;
  cout<<"enter the localhost address that you want: "<<endl;
  getline(cin,localhostNumber);
  getaddrinfo(0,localhostNumber.c_str(),&hints,&bind_address);
  // now we have our address setup in bind_address
  //next step is to create socket :)
  cout<<"Creating socket is in progress..."<<endl;
  SOCKET socket_listen;
  //now we use socket() to create this baby ...
  socket_listen = socket(
    bind_address -> ai_family,
    bind_address -> ai_socktype,
    bind_address -> ai_protocol
    );
  // creating a condition that indicates if there`s any problem
  if(!ISVALIDSOCKET(socket_listen)){
    cout<<"sock() func failed"<<stderr<<GETSOCKETERRNO();
    return 1;
  }
  // after creating socket it`s time to bind the socket to associate socket to our address
  cout<<"binding to address ..."<<endl;
  if(bind(socket_listen,bind_address -> ai_addr , bind_address ->ai_addrlen)){
    cout<<"there was a problem in bind() func..."<<GETSOCKETERRNO()<<endl;
    return 1;
  }
  freeaddrinfo(bind_address);
  // now is the time that our server listes to a particular port 
  cout<<"Listening..."<<endl;
  if(listen(socket_listen,10) < 0){
    cout<<"listening has failed"<<GETSOCKETERRNO()<<endl;
    return 1;
  }
  // now our server ambushes and wait for connections :))
  cout<<"server is waiting for connections:::"<<endl;
  //before accepting connection from client we need to declare 
  //new sockaddr_storage variable to store the addressinfo for the connecting conected client
  //we must pass another arg to accept() which is client_address_len which is dependant on IPv version
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  // we pass the result of accept to new socket :)
  SOCKET socket_client =
   accept(
    socket_listen,
    (struct sockaddr *)&client_address,
    &client_len
  );
  //making sure socket is valid
  if(!ISVALIDSOCKET(socket_client)){
    cout<<"there`s somthing wrong with socket client and accept():::"<<GETSOCKETERRNO()<<endl;
    return 1;
  }
  // now magic begans :))
  cout<<"client is connecting:::"<<endl;
  char address_buffer [100];
  //we call getnameinfo to convert socket address to a corresponding host and service
  getnameinfo(
    (struct sockaddr*) &client_address,
    client_len,
    address_buffer,
    sizeof(address_buffer),
    0,
    0,
    NI_NUMERICHOST);
    cout<<"address is : "<<address_buffer<<endl;
    cout<<"reading request:::"<<endl;
    char request[1024];
    int bytes_received = recv(socket_client,request,1024,0);
    cout<<"recived bytes : "<<bytes_received<<endl;
    cout<<bytes_received<<" "<<request<<endl;
    cout<<"Sending response:::"<<endl;
    const char *response = 
      "HTTP/1.1 200 OK\r\n"
      "Connection: close\r\n"
      "Content-Type: text/plain\r\n\r\n"
      "Local time is: ";
    int bytes_sent = send(socket_client,response,strlen(response),0);
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    cout<<"bytes sent of bytes: "<<bytes_sent<<" "<<(int)strlen(time_msg);
    cout<<"closing connection...."<<endl;
    CLOSESOCKET(socket_client);
    cout<<"closing listening socket (servers socket) "<<endl;
    CLOSESOCKET(socket_client);
#if defined(_WIN32)
  WSACleanup();
#endif
    cout<<":FinisheD:"<<endl;
    return 0;
}