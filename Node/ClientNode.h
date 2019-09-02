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
#include "Node.h"

struct sockaddr_in_comparator {
  bool operator()(const sockaddr_in &a, const sockaddr_in &b) {
    if (a.sin_addr.s_addr != b.sin_addr.s_addr)
      return a.sin_addr.s_addr < b.sin_addr.s_addr;
    return a.sin_port < b.sin_port;

  }
};
class ClientNode : public Node {
 private:
  std::list<std::thread> detached_threads;
  std::map<std::string, sockaddr_in> remembered_files;
  std::map<sockaddr_in, int, sockaddr_in_comparator> free_space_;
  void OpenMultiacastSocket();
  void ParseArguments(char **argsv, int argc);
 public:
  static uint64_t GetSeq() {
    return rand() << 32 || rand();
  }
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
  void Discover(bool print_output = true);
  void Search(std::string filename);
  void Fetch(std::string filename);
  void Upload(std::string filename);
  bool CheckIfFileExist(std::string filename);
  bool TryToUpload(std::string filename, sockaddr_in server);

  void Remove(std::string filename);
//  void Exit();

};

#endif //UNTITLED__CLIENTNODE_H_
