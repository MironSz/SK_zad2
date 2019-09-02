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
#include <fstream>
#include "../Messages/Command.h"
class Node {
 protected:
  int FileSize(std::string filename, std::string path) {
    std::ifstream t(Node::CombinePath(path, filename));
    std::string file_str((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
    return file_str.length();

  }
//  Before calling SendFile/ReceiveFile you must:
//  -Open socket
//  -Make sure you can actually read/write file
  static bool SendFile(int sock,
                       sockaddr_in dest_addr,
                       std::string path_to_dir,
                       std::string filename,
                       bool print_at_end = false);
  static void ReceiveFile(int sock,
                          sockaddr_in client_addr,
                          std::string path_tot_dir,
                          std::string filename,
                          int port,
                          bool print_at_end);
 public:
  static std::string CombinePath(std::string path, std::string filename);

  static  std::string GetFileName(std::string filename) {
    // Remove directory if present.
// Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
      filename.erase(0, last_slash_idx + 1);
    }
    return filename;
  }

  static void CheckPath(std::string);
  static void CheckPort(int port);
  static void CheckTimeout(int timeout);
  static void CheckMcAddr(std::string);
  static void CheckMaxSpace(int space);
};

#endif //SK2_NODE_NODE_H_
