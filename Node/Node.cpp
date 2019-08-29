//
// Created by miron on 28.08.2019.
//

#include "Node.h"
#include "../Lib.h"
#include <iostream>
#include <cctype>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <regex>
#include <netdb.h>
#include <fstream>
#include "dirent.h"
void Node::SendFile(int sock,
                    sockaddr_in dest_addr,
                    std::string path_to_dir,
                    std::string filename) {
  int listen_result = listen(sock,10);
  log_message("Sending " + path_to_dir + "/" + filename);
  std::ifstream t(path_to_dir + "/" + filename);
  std::string file_str((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
  log_message("File contents\n" + file_str);
  int already_send_bytes = 0;
  int sent_bytes = 1;
  while ((unsigned int) already_send_bytes < file_str.length() && sent_bytes > 0) {
    log_message("Sending tcp packet");
    sent_bytes = send(sock,
                        file_str.c_str() + already_send_bytes,
                        file_str.length() - already_send_bytes,
                        0);
    if (sent_bytes) {
//      TODO: handle error
      log_message("Unable to send tcp packet");
      close(sock);
      return;
    } else {
      log_message("Sent tcp packet");
    }
    already_send_bytes += sent_bytes;
  }
  log_message("Finished sending tcp packets");
  close(sock);

}
void Node::ReceiveFile(int sock,
                       sockaddr_in client_addr,
                       std::string path_to_dir,
                       std::string filename) {
  char buffer[1000];
  socklen_t addr_len = sizeof(sockaddr_in);

  log_message("Receiving file, writing to " + path_to_dir + "/" + filename);
  std::ofstream output_stream(path_to_dir + "/" + filename);
  int number_of_received_bytes;
  do {
    number_of_received_bytes =
        recv(sock, buffer, 1000, 0);
    if (number_of_received_bytes > 0) {
      std::string received_bytes(buffer, number_of_received_bytes);
      log_message("Writing (" + received_bytes + ")to file");
      output_stream << received_bytes;
    }
  } while (number_of_received_bytes > 0);
  close(sock);
}
