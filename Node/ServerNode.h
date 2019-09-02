//
// Created by miron on 21.08.2019.
//

#ifndef UNTITLED__SERVERNODE_H_
#define UNTITLED__SERVERNODE_H_
#include <string>
#include <vector>
#include <list>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include "../Messages/Command.h"
#include "../Messages/ComplexCommand.h"
#include "Node.h"
class ServerNode : public Node {
 private:
  std::list<std::thread> detached_threads_;
  ip_mreq ip_mreq_;
  std::string ip;
  std::vector<std::string> files;
  void OpenMulticastSocket();
  void ParseArguments(char **argsv, int argc);
 public:
  sockaddr_in client_address_;
  sockaddr_in server_address_;
  std::string mcast_addr_;
  int multicast_socket_;
  int cmd_port_;
  int timeout_;
  int free_space_;
  std::string path_to_folder_;

  ServerNode(char **argsv, int argc);
  void StartWorking();
  void IndexFiles();
  void GetGlobalLock();
  void FreeGlobalLock();
//  void ConnectMcaddr();
  void Discover(Command *command);
  void Fetch(Command *command);
  void Search(Command *command);
  bool CheckIfFileExists(std::string filename);
  void Upload(ComplexCommand *command);
  void Remove(Command *command);
};

#endif //UNTITLED__SERVERNODE_H_
