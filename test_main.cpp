//
// Created by miron on 22.08.2019.
//
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
int main( int argc,char ** argv){
  int sock, optval;
  int timeout = 10;
  struct sockaddr_in client_address;
  struct sockaddr_in server_address;;
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    throw "Unable to open socket";

  optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof(int)) < 0)
    throw "setsockopt(SO_REUSEADDR) failed";
  optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval,
                 sizeof optval) < 0)
    throw "setsockopt broadcast";

  optval = 100;
  if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval,
                 sizeof optval) < 0)
    throw "setsockopt multicast ttl";
  struct timeval opttime;
  opttime.tv_sec = timeout;
  opttime.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *) &opttime, sizeof opttime) < 0)
    throw ("setsockopt receive timeout_");

  /* podpięcie się do grupy rozsyłania (ang. multicast) */
  struct ip_mreq mreq;
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  std::string mcast_addr = "230.200.120.1";
  if (inet_aton(mcast_addr.c_str(), &mreq.imr_multiaddr) == 0)
    throw ("inet_aton");
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &mreq,
                 sizeof (ip_mreq)) < 0)
    throw ("setsockopt");

  client_address.sin_family = AF_INET;
  client_address.sin_addr.s_addr = htonl(INADDR_ANY);
  client_address.sin_port = htons(0);

  server_address.sin_family = AF_INET;
  int cmd_port = 1234;
  server_address.sin_port = htons(cmd_port);
  if (inet_aton(mcast_addr.c_str(), &server_address.sin_addr) == 0)
    throw ("inet_aton");
  if (bind(sock, (struct sockaddr *) &client_address, sizeof(client_address)) < 0)
    throw "bind";
}
