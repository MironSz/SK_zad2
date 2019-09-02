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
bool Node::SendFile(int sock,
                    sockaddr_in dest_addr,
                    std::string path_to_dir,
                    std::string filename,
                    bool print_at_end) {
  std::string pure_path_to_file;
  if (path_to_dir.empty())
    pure_path_to_file = filename;
  else
    pure_path_to_file = path_to_dir + "/" + filename;
  log_message(pure_path_to_file);
  std::ifstream t(pure_path_to_file);
  std::string file_str((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
  log_message("File contents:\n" + file_str);
  int already_send_bytes = 0;
  int sent_bytes = 1;
  while ((unsigned int) already_send_bytes < file_str.length() && sent_bytes > 0) {
    log_message(
        "Sending tcp packet " + std::to_string(file_str.length() - already_send_bytes) + "    "
            + (file_str.c_str() + already_send_bytes));
    sent_bytes = write(sock,
                       file_str.c_str() + already_send_bytes,
                       file_str.length() - already_send_bytes);
    if (sent_bytes <= 0) {
      log_message("Unable to send tcp packet");
    } else {
      log_message("Sent tcp packet");
      already_send_bytes += sent_bytes;
    }
  }
  if ((unsigned int) already_send_bytes < file_str.length()) {
    log_message("Couldnt finish sending file");
    printf("File %s uploading failed (%s:%hu) server didn't receive all bytes\n",
           GetFileName(filename).c_str(),
           inet_ntoa(dest_addr.sin_addr),
           ntohs(dest_addr.sin_port));
    return false;
  }

  printf("File %s uploaded (%s:%hu)\n",
         GetFileName(filename).c_str(),
         inet_ntoa(dest_addr.sin_addr),
         ntohs(dest_addr.sin_port));

  log_message("Finished sending tcp packets " + std::to_string(already_send_bytes));
  close(sock);
  return true;
}
void Node::ReceiveFile(int sock,
                       sockaddr_in client_addr,
                       std::string path_to_dir,
                       std::string filename,
                       int port,
                       bool print_at_end) {
  char buffer[1000];

  log_message("Receiving file, writing to " + path_to_dir + "/" + filename);
  std::ofstream output_stream(path_to_dir + "/" + filename, std::ofstream::binary);
  int number_of_received_bytes = 5;
  do {
    number_of_received_bytes =
        read(sock, buffer, 999);
    log_message("Received package " + std::to_string(number_of_received_bytes));
    if (number_of_received_bytes > 0) {
      std::string received_bytes(buffer, number_of_received_bytes);
      log_message("Writing (" + received_bytes + ") to file");
      output_stream.write(buffer, number_of_received_bytes);
    }
  } while (number_of_received_bytes > 0);
  log_message("Finished reading file");
  if (print_at_end) {
    printf("File %s downloaded (%s:%d)\n",
           (filename).c_str(),
           inet_ntoa(client_addr.sin_addr),
           port);
  }
  close(sock);
}
std::string Node::CombinePath(std::string path, std::string filename) {
  if (filename[0] == '/' || path == "") {
    return filename;
  }
  path += "/" + filename;
  /* Locate the substring to replace. */
  int index = 0;
  index = path.find("/./", index);
  if (index == std::string::npos)
    return path;
  /* Make the replacement. */
  path.replace(index, 3, "/");
  /* Advance index forward so the next iteration doesn't pick it up as well. */
  return path;
}
void Node::CheckPath(std::string path) {
  DIR *dir = opendir(path.c_str());
  if (dir) {
    closedir(dir);
  } else
    throw std::runtime_error("Wrong directory");
}
void Node::CheckPort(int port) {

}
void Node::CheckTimeout(int timeout) {
  if (timeout <= 0 || timeout > 300)
    throw std::runtime_error("Wrong timeout");

}
void Node::CheckMcAddr(std::string addr) {
//TODO
}
void Node::CheckMaxSpace(int space) {
  if (space <= 0)
    throw std::runtime_error("Wrong max space");

}
