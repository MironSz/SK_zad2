//
// Created by miron on 21.08.2019.
//

#include "../Lib.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <cctype>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <regex>
#include <netdb.h>
#include <fstream>
#include "dirent.h"

using namespace boost::program_options;

#include "../Lib.h"
#include "../Messages/SimpleCommand.h"
#include "../Messages/ComplexCommand.h"
#include "ServerNode.h"
#define DEFAULT_SPACE 1000
//#define DEFAULT_SPACE 52428800



void ServerNode::OpenMulticastSocket() {
  int optval;
  /* otworzenie gniazda */

  multicast_socket_ = socket(AF_INET, SOCK_DGRAM, 0);

  if (multicast_socket_ < 0) {
    throw "Opening socket";
  }

  /*Wiele serweróœ na jednej maszynie */
//    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0)
//        syserr("setsockopt(SO_REUSEADDR) failed");
  /*Podpięcie się do grupy rozsyłania (ang. multicast) */


  optval = 1;
  if (setsockopt(multicast_socket_, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval) < 0)
    throw ("setsockopt broadcast");
  optval = 2;
  if (setsockopt(multicast_socket_, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval) <
      0)
    throw ("setsockopt multicast ttl");

  ip_mreq_.imr_interface.s_addr = htobe64(INADDR_ANY);
  if (inet_aton(mcast_addr_.c_str(), &ip_mreq_.imr_multiaddr) == 0)
    throw ("inet_aton");
  if (setsockopt(multicast_socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq_,
                 sizeof ip_mreq_) < 0)
    throw ("setsockopt add membership");
  optval = 1;
  if (setsockopt(multicast_socket_, IPPROTO_IP, IP_PKTINFO, &optval, sizeof(optval)) < 0) {
    throw ("setsockopt");
  }
  server_address_.sin_family = AF_INET; // IPv4
  server_address_.sin_addr.s_addr = htobe64(INADDR_ANY); // listening on all interfaces
  server_address_.sin_port = htons(cmd_port_); // default port for receiving is PORT_NUM

  if (bind(multicast_socket_, (sockaddr *) &server_address_, sizeof(server_address_)) < 0)
    throw ("bind");
}
void ServerNode::ParseArguments(char **argv, int argc) {
  options_description desc{"Options"};
  desc.add_options()
      ("g", value<std::string>()->required(), "Adres rozglaszania ukierunkowanego")
      ("p", value<int>()->required(), "Port UDP")
      ("b", value<int>()->required()->default_value(DEFAULT_SPACE), "Max shared space")
      ("f", value<std::string>()->required(), "Path to shared folder")
      ("t", value<int>()->default_value(5), "Timeout");

  variables_map vm;
  store(command_line_parser(argc, argv).options(desc).style(
      command_line_style::long_allow_adjacent |
          command_line_style::short_allow_adjacent |
          command_line_style::allow_long_disguise).run(), vm);

  notify(vm);

  if (vm.count("g")) {
    mcast_addr_ = vm["g"].as<std::string>();
  }
  if (vm.count("p")) {
    cmd_port_ = vm["p"].as<int>();
  }
  if (vm.count("b")) {
    free_space_ = vm["b"].as<int>();
  }
  if (vm.count("f")) {
    path_to_folder_ = vm["f"].as<std::string>();
  }
  if (vm.count("t")) {
    timeout_ = vm["t"].as<int>();
  }
}
ServerNode::ServerNode(char **argv, int argc) {
  ParseArguments(argv, argc);
  OpenMulticastSocket();
//  AlternativeOpenSocket();
  IndexFiles();

  char hostbuffer[100];
  char *IPbuffer;
  struct hostent *host_entry;
  int hostname;
  // To retrieve hostname
  hostname = gethostname(hostbuffer, 30);

  // To retrieve host information
  host_entry = gethostbyname(hostbuffer);
  // To convert an Internet network
  // address into ASCII string

  IPbuffer = inet_ntoa(*((struct in_addr *)
      host_entry->h_addr_list[0]));
  ip = std::string(IPbuffer);
  std::cout << ip << "\n";
}
void ServerNode::StartWorking() {
  log_message("Started working");

  while (true) {
    Command *request = Command::ReadCommand(multicast_socket_,
                                            0,
                                            reinterpret_cast< sockadrr_in *>(&client_address_),
                                            0,
                                            "",
                                            sizeof(sockaddr_in));
    log_message("Received request " + request->GetCommand());

    if (request->GetCommand() == "HELLO") {
      Discover(request);
    } else if (request->GetCommand() == "LIST") {
      Search(request);
    } else if (request->GetCommand() == "GET") {
      Fetch(request);
    } else if (request->GetCommand() == "SERVER_EMERGENCY_SHUTDOWN_PROTOCOL_OVER_9000") {
      break;
    }

  }

}
void ServerNode::IndexFiles() {
  DIR *dir;
  dirent *ent;
  log_message("Indexing files in " + path_to_folder_);
  if ((dir = opendir(path_to_folder_.c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr) {
      if (ent->d_type == DT_REG) {
        std::string filename(ent->d_name);
        files.push_back(filename);
        log_message("Indexed " + filename);
      }
    }
    closedir(dir);
  } else {
    log_message("Could not open the directory");
//    TODO: Could not open dir
  }
}
void ServerNode::Discover(Command *command) {
  log_message("Started Discover");

  std::string good = "GOOD_DAY";
  ComplexCommand response(good, command->GetSeq(), (uint64_t) free_space_, ip);
  auto h = response.GetData();
  log_message("Sending response");

  response.SendTo(multicast_socket_,
                  0,
                  reinterpret_cast<const sockaddr *>(&client_address_),
                  sizeof(client_address_));
  log_message("Sent response");

  log_message("Finished Discover");

}
void ServerNode::Search(Command *command) {
  log_message("Started Search, looking for " + command->GetData());
  std::string my_list = "MY_LIST";
  std::string inner_buffer = "";
  for (auto &file : files) {
    if (file.find(command->GetData()) != std::string::npos) {
      if (file.length() + inner_buffer.length() + 1
          <= BUFFER_SIZE - CMD_LENGTH - sizeof(uint64_t)) {
        inner_buffer += ("\n" + file);
      } else {
        log_message("Sending package");

        SimpleCommand response(my_list, command->GetSeq(), inner_buffer);
        response.SendTo(multicast_socket_,
                        0,
                        reinterpret_cast<const sockaddr *>(&client_address_),
                        sizeof(client_address_));
        inner_buffer = "";
      }
    }
  }
  if (!inner_buffer.empty()) {
    log_message("Sending final package");

    SimpleCommand response(my_list, command->GetSeq(), inner_buffer);
    response.SendTo(multicast_socket_,
                    0,
                    reinterpret_cast<const sockaddr *>(&client_address_),
                    sizeof(client_address_));
    inner_buffer = "";
  }
  log_message("Finished Search");
}

void ServerNode::Fetch(Command *command) {
  std::string filename = command->GetData();
  log_message("Fetch " + filename);
  if (!CheckIfFileExists(filename)) {
    log_message("Such file does not exist");
  }
  detached_threads_.emplace_back(SendFile,
                                 command,
                                 client_address_,
                                 multicast_socket_,
                                 path_to_folder_);
}
bool ServerNode::CheckIfFileExists(std::string filename) {
  for (auto &file : files) {
    if (file == filename) {
      return true;
    }
  }
  return false;
}
void ServerNode::SendFile(Command *command,
                          sockaddr_in client_addr,
                          int multicast_socket,
                          std::string path_to_input_dir) {
  log_message("Sending file in new thread " + command->GetData() + " " + path_to_input_dir);
  sockaddr_in server_address;
  std::string filename = command->GetData();
  std::string s_connect_me = "CONNECT_ME";
  int sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket

  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); //
  server_address.sin_port = htons(0); // listening on port PORT_NUM
  socklen_t addr_len = sizeof(client_addr);
  if (bind(sock, reinterpret_cast<sockaddr * > (&server_address), addr_len) < 0 ||
      getsockname(sock, reinterpret_cast<sockaddr * > (&server_address), &addr_len) < 0) {
//    TODO:
    log_message("Could not open socket");
  }
  uint64_t opened_port = server_address.sin_port;
  log_message("Opened port " + std::to_string(opened_port));

  ComplexCommand connect_me(s_connect_me, command->GetSeq(), opened_port, filename);

  connect_me.SendTo(multicast_socket,
                    0,
                    reinterpret_cast<sockaddr *> (&client_addr),
                    sizeof(client_addr));
  log_message("Sent \"CONNECT_ME\" message");

  std::ifstream t(path_to_input_dir + "/" + filename);
  std::string file_str((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
  log_message("Opened file:\n" + file_str);
  int already_send_bytes = 0;
  int sent_bytes = 1;
  while (already_send_bytes < file_str.length() && sent_bytes > 0) {
    log_message("Sending tcp packet");
    sent_bytes = sendto(sock,
                        file_str.c_str() + already_send_bytes,
                        file_str.length() - already_send_bytes,
                        0,
                        reinterpret_cast<sockaddr *>(&client_addr),
                        sizeof(client_addr));
    if (sent_bytes) {
//      TODO: handle error
      log_message("Unable to send tcp packet");
    } else {
      log_message("Sent tcp packet");
    }
    already_send_bytes += sent_bytes;
  }
  log_message("Finished sending tcp packets");

}




