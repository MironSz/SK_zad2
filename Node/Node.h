//
// Created by miron on 28.08.2019.
//

#ifndef SK2_NODE_NODE_H_
#define SK2_NODE_NODE_H_
#include <string>
#include <vector>
#include <list>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include "../Messages/Command.h"
class Node {
 protected:
//  Before calling SendFile/ReceiveFile you must:
//  -Open socket
//  -Make sure you can actually read/write file
  static void SendFile(int sock,
                       sockaddr_in dest_addr,
                       std::string path_to_dir,
                       std::string filename);
  static void ReceiveFile(int sock,
                          sockaddr_in client_addr,
                          std::string path_tot_dir,
                          std::string filename);
};

#endif //SK2_NODE_NODE_H_
