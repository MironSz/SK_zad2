//
// Created by miron on 21.08.2019.
//
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <cctype>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <regex>
#include <poll.h>
using namespace boost::program_options;

#include "ClientNode.h"
#include "../Lib.h"
#include "../Messages/SimpleCommand.h"
#include "../Messages/ComplexCommand.h"
void ClientNode::OpenMultiacastSocket() {
  int optval;
  multicast_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (multicast_socket_ < 0)
    throw "Unable to open socket";

  optval = 1;
  if (setsockopt(multicast_socket_, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof(int)) < 0)
    throw "setsockopt(SO_REUSEADDR) failed";
  optval = 1;
  if (setsockopt(multicast_socket_, SOL_SOCKET, SO_BROADCAST, (void *) &optval,
                 sizeof optval) < 0)
    throw "setsockopt broadcast";

  optval = 100;
  if (setsockopt(multicast_socket_, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval,
                 sizeof optval) < 0)
    throw "setsockopt multicast ttl";
  struct timeval opttime;
  opttime.tv_sec = timeout_;
  opttime.tv_usec = 0;

  if (setsockopt(multicast_socket_, SOL_SOCKET, SO_RCVTIMEO, (void *) &opttime, sizeof opttime) < 0)
    throw ("setsockopt receive timeout_");

  /* podpięcie się do grupy rozsyłania (ang. multicast) */
  struct ip_mreq mreq;
  mreq.imr_interface.s_addr = htobe64(INADDR_ANY);
  if (inet_aton(mcast_addr_.c_str(), &mreq.imr_multiaddr) == 0)
    throw ("inet_aton");
  if (setsockopt(multicast_socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &mreq,
                 sizeof(ip_mreq)) < 0)
    throw ("setsockopt");

  client_address_.sin_family = AF_INET;
  client_address_.sin_addr.s_addr = htobe64(INADDR_ANY);
  client_address_.sin_port = htons(0);

  server_address_.sin_family = AF_INET;
  server_address_.sin_port = htons(cmd_port_);
  if (inet_aton(mcast_addr_.c_str(), &server_address_.sin_addr) == 0)
    throw ("inet_aton");
  if (bind(multicast_socket_, (struct sockaddr *) &client_address_, sizeof(client_address_)) < 0)
    throw "bind";
}

ClientNode::ClientNode(char **argsv, int argc) {
  ParseArguments(argsv, argc);
  OpenMultiacastSocket();
}

void ClientNode::ParseArguments(char **argv, int argc) {
  options_description desc{"Options"};
  desc.add_options()
      ("g",
       value<std::string>()->required()->notifier(Node::CheckMcAddr),
       "Adres rozglaszania ukierunkowanego")
      ("p", value<int>()->required()->notifier(Node::CheckPort), "Port UDP")
      ("o", value<std::string>()->required()->notifier(Node::CheckPath), "Path to files")
      ("t", value<int>()->default_value(5)->notifier(Node::CheckTimeout), "Timeout");
//      ("g", value<std::string>()->notifier(&check_mc_addrs), "Adres rozglaszania ukierunkowanego")
//      ("p", value<int>()->required()->notifier(&check_port), "Port UDP")
//      ("o", value<std::string>()->required()->notifier(&check_fld), "Path to files")
//      ("t", value<int>()->default_value(5)->notifier(&check_timeout), "Timeout");

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
  if (vm.count("o")) {
    path_to_folder_ = vm["o"].as<std::string>();
  }
  if (vm.count("t")) {
    timeout_ = vm["t"].as<int>();
  }
}

void ClientNode::StartWorking() {
  std::regex exit("^exit$");
  std::regex discover("^discover$");
  std::regex fetch("^fetch\\s+(\\w*)$");
  std::regex search("^search\\s+(\\w*)$");
  std::regex remove("^remove\\s+(\\w+)$");
  std::regex upload("^upload\\s+[^\\s]+$");
  std::regex argument("(\\w*)$");
  std::regex upload_argument("([^\\s]+)$");
  while (true) {
    log_message("Awaiting instruction");
    std::smatch matched;

    std::string command;
    std::getline(std::cin, command);
    log_message("Instruction (" + command + ")");

    if (std::regex_search(command, matched, exit)) {
      break;
    } else if (std::regex_search(command, matched, discover)) {
      Discover();
    } else if (std::regex_search(command, matched, fetch)) {
      std::regex_search(command, matched, argument);
      Fetch(matched[0]);
    } else if (std::regex_search(command, matched, search)) {
      std::regex_search(command, matched, argument);
      Search(matched[0]);
    } else if (std::regex_search(command, matched, remove)) {
      std::regex_search(command, matched, argument);
      Remove(matched[0]);
    } else if (std::regex_search(command, matched, upload)) {
      log_message("uploading ");
      std::regex_search(command, matched, upload_argument);
      Upload(matched[0]);
    } else if (command == "exit") {
      return;
    }
  }

}

void ClientNode::Discover(bool print_output) {
  log_message("Discover begin");
  uint64_t send_seq = GetSeq();// random_seq();
  SimpleCommand request_message(std::string("HELLO"), send_seq, std::string(""));

  request_message.SendTo(multicast_socket_,
                         0,
                         server_address_,
                         sizeof(server_address_));

  log_message("Sent message");

  ComplexCommand response_message(multicast_socket_,
                                  0,
                                  server_address_,
                                  send_seq,
                                  "GOOD_DAY");

  while (response_message.GetLen() > 0) {
    log_message("Processing response " + response_message.GetCommand());
    if (print_output)
      printf("Found %s (%s) with free space %lu\n", inet_ntoa(server_address_.sin_addr),
             response_message.GetData().c_str(),
             response_message.GetParam());
    free_space_[server_address_] = response_message.GetParam();
    response_message = ComplexCommand(multicast_socket_,
                                      0,
                                      server_address_,
                                      send_seq,
                                      "GOOD_DAY");
  }
  log_message("Discover end");
}
void ClientNode::Search(std::string filename) {
  uint64_t seq = GetSeq();
  SimpleCommand request("LIST", seq, filename);
  request.SendTo(multicast_socket_,
                 0,
                 server_address_,
                 sizeof(server_address_));
  sockaddr_in last_server;
  SimpleCommand response_message(multicast_socket_,
                                 0,
                                 last_server,
                                 seq,
                                 "MY_LIST");
  while (response_message.GetLen() > 0) {
    std::stringstream data(response_message.GetData());
    std::string filename;
    while (std::getline(data, filename, '\n')) {
      if (filename.length() > 0) {
        remembered_files[filename] = last_server;
        printf("%s (%s)\n", filename.c_str(), inet_ntoa(last_server.sin_addr));
      }
    }

    response_message = SimpleCommand(multicast_socket_,
                                     0,
                                     last_server,
                                     seq,
                                     "MY_LIST");
  }

}
void ClientNode::Fetch(std::string filename) {
  if (remembered_files.find(filename) == remembered_files.end()) {
    log_message("Could  not find the file\n");
    return;
  }
  uint64_t seq_nr = GetSeq();
  sockaddr_in server_addr = server_address_;
  SimpleCommand request("GET", seq_nr, filename);
  request.SendTo(multicast_socket_,
                 0,
                 remembered_files[filename],
                 sizeof(sockaddr_in));

  ComplexCommand response(multicast_socket_,
                          0,
                          server_addr,
                          seq_nr,
                          "CONNECT_ME");

  if (response.GetLen() == 0) {
    log_message("Did not receive response");
    return;
  }
  log_message("Received response " + response.GetCommand() + " " + response.GetData() + "  "
                  + std::to_string(response.GetParam()));

  int sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
  socklen_t addr_len = sizeof(sockaddr_in);
  server_addr.sin_port = htons(response.GetParam()); // listening on port PORT_NUM


  log_message("Connecting to server on " + std::to_string(server_addr.sin_port) + " "
                  + std::to_string(server_addr.sin_addr.s_addr));

  int connect_result = connect(sock, reinterpret_cast<sockaddr *>(&server_addr), addr_len);
  if (connect_result < 0) {
    log_message("Couldnt connect");
    return;
  } else {
    log_message("Connected");
  }

  struct timeval tv;

  tv.tv_sec = timeout_;  /* 30 Secs Timeout */
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval));
//  ReceiveFile(sock, server_addr, path_to_folder_, filename);
  detached_threads.emplace_back(Node::ReceiveFile,
                                sock,
                                server_addr,
                                path_to_folder_,
                                filename,
                                response.GetParam(),
                                true);
}

bool ClientNode::TryToUpload(std::string filename, sockaddr_in server) {

  uint64_t seq = ClientNode::GetSeq();
  log_message("Trying to upload file");
  std::string add_me_s = "ADD";
  std::string filename_pure = GetFileName(filename);
  ComplexCommand add_me(add_me_s, seq, FileSize(filename, path_to_folder_), filename_pure);

  add_me.SendTo(multicast_socket_, 0, server, sizeof(sockaddr_in));

  Command
      *response = Command::ReadCommand(multicast_socket_, 0, server, seq, "", sizeof(sockaddr_in));
  if (response->GetCommand() == "NO_WAY") {
    log_message("Server said no way");
    return false;
  } else if (response->GetCommand() != "CAN_ADD") {
    return false;
  }
  ComplexCommand *can_add = reinterpret_cast<ComplexCommand *>(response);
  if (can_add->GetLen() <= 0) {
    printf("File %s uploading failed (%s:) server didn't respond",
           GetFileName(filename).c_str(),
           inet_ntoa(server.sin_addr));
    return true;
  }
  uint64_t opened_port = can_add->GetParam();

  log_message("Received response " + can_add->GetCommand() + " " + can_add->GetData() + "  "
                  + std::to_string(can_add->GetParam()));

  int sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
  socklen_t addr_len = sizeof(sockaddr_in);
  server.sin_port = htons(can_add->GetParam()); // listening on port PORT_NUM


  log_message("Connecting to server on " + std::to_string(server.sin_port) + " "
                  + std::to_string(server.sin_addr.s_addr));
  timeval tv;
  tv.tv_sec = timeout_;  /* 30 Secs Timeout */
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(timeval));

  int connect_result = connect(sock, reinterpret_cast<sockaddr *>(&server), addr_len);
  if (connect_result < 0) {
    log_message("Couldnt connect");

    printf("File %s uploading failed (%s:%lu) server didn't accept connection\n",
           GetFileName(filename).c_str(),
           inet_ntoa(server.sin_addr),
           opened_port);
    return true;
  } else {
    log_message("Connected");
  }

  detached_threads.emplace_back(Node::SendFile, sock, server, path_to_folder_, filename, true);

  return true;
}

void ClientNode::Upload(std::string filename) {
  bool success = false;
  log_message("Begin Upload");
  log_message(GetFileName(filename));
  log_message("Begin Upload");
  sockaddr_in chosen_server;
  if (!CheckIfFileExist(filename)) {
    printf("File %s does not exist\n", GetFileName(filename).c_str());
    return;
  }
  Discover(false);
  for (auto &desc:free_space_) {
    if (desc.second >= FileSize(filename, "")
        && TryToUpload(filename, desc.first)) {
      success = true;
      break;
    }
  }
  if (!success) {
    printf("File %s to big\n", filename.c_str());
  }
}

bool ClientNode::CheckIfFileExist(std::string filename) {
  log_message("Checking if " + CombinePath(path_to_folder_, filename) + " exists");
  std::ifstream f(CombinePath(path_to_folder_, filename).c_str());
  std::ifstream f2(filename.c_str());
  return f.good() || f2.good();
}
void ClientNode::Remove(std::string filename) {
  log_message("Removing " + filename);
  SimpleCommand("DEL", GetSeq(), filename).SendTo(multicast_socket_,
                                                  0,
                                                  server_address_,
                                                  sizeof(server_address_));
}



