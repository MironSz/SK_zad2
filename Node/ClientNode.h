//
// Created by miron on 21.08.2019.
//

#ifndef UNTITLED__CLIENTNODE_H_
#define UNTITLED__CLIENTNODE_H_
#include <string>
#include <map>
#include <list>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include "../Messages/ComplexCommand.h"
class ClientNode {
 private:
  std::list<std::thread> detached_threads;
  std::map<std::string, sockaddr_in> remembered_files;
  void OpenMultiacastSocket();
  void ParseArguments(char **argsv, int argc);
 public:
  struct sockaddr_in client_address_;
  struct sockaddr_in server_address_;
  std::string mcast_addr_;
  int multicast_socket_;
  int cmd_port_;
  int timeout_;
  std::string path_to_folder_;

  ClientNode(char **argsv, int argc);
  void StartWorking();
//  void IndexFiles();
//  void ConnectMcaddr();
  void Discover();
  void Search(std::string filename);
  void Fetch(std::string filename);
//  void Upload(std::string filename);
//  void Remove(std::string filename);
//  void Exit();
  static void ReceiveFile(ComplexCommand command,
                          sockaddr_in server_addr,
                          std::string path_to_dir,
                          std::string filename);
};

#endif //UNTITLED__CLIENTNODE_H_
