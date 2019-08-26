//
// Created by miron on 21.08.2019.
//
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

void ClientNode::Search(std::string filename) {

}
void ClientNode::Discover() {
  log_message("Discover begin");
  uint64_t send_seq = 1;// random_seq();
  SimpleCommand request_message(std::string("HELLO"), send_seq, std::string(""));
  log_message("Prepared message");

  request_message.SendTo(multicast_socket_,
                         0,
                         reinterpret_cast<const sockaddr *>(&server_address_),
                         sizeof(server_address_));

  log_message("Sent message");

//  int rcva_len = sizeof(client_address_);
  ComplexCommand response_message(multicast_socket_,
                                  0,
                                  reinterpret_cast<struct sockadrr_in *>(&server_address_),
                                  send_seq,
                                  sizeof(client_address_));

  while (response_message.GetLen() > 0) {
    log_message("Processing response " + response_message.GetCommand());

    printf("Found %s (%s) with free space %lu\n", response_message.GetData().c_str(),
           inet_ntoa(server_address_.sin_addr), response_message.GetParam());

    response_message = ComplexCommand(multicast_socket_,
                                      0,
                                      reinterpret_cast<struct sockadrr_in *>(&server_address_),
                                      send_seq);

  }
  log_message("Discover end");
}

ClientNode::ClientNode(char **argsv, int argc) {
  ParseArguments(argsv, argc);
  OpenMultiacastSocket();
}

void ClientNode::ParseArguments(char **argv, int argc) {
  options_description desc{"Options"};
  desc.add_options()
      ("g", value<std::string>()->required(), "Adres rozglaszania ukierunkowanego")
      ("p", value<int>()->required(), "Port UDP")
      ("o", value<std::string>()->required(), "Path to files")
      ("t", value<int>()->default_value(5), "Timeout");
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
  std::regex fetch("^fetch (\\w*)$");
  std::regex argument(" (\\w*)$");
  while (true) {
    std::smatch matched;

    std::string command;
    std::getline(std::cin, command);

    if (std::regex_search(command, matched, exit)) {
      std::cout << matched[0];
      break;
    } else if (std::regex_search(command, matched, discover)) {
      Discover();
    } else if (std::regex_search(command, matched, fetch)) {
      std::regex_search(command, matched, argument);
      std::cout << "fetch (" << matched[0] << ")\n";
    }

  }

}
