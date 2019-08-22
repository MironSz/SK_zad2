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
#include <sys/socket.h>
#include <regex>
using namespace boost::program_options;

#include "ClientNode.h"
#include "Lib.h"
void ClientNode::OpenMultiacastSocket() {
  int sock, optval;
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    throw "Unable to open socket";

  optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof(int)) < 0)
    throw "setsockopt(SO_REUSEADDR) failed";
  optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval,
                 sizeof optval) < 0)
    throw "setsockopt broadcast";

  optval = 100;
  if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval,
                 sizeof optval) < 0)
    throw "setsockopt multicast ttl";
  struct timeval opttime;
  opttime.tv_sec = timeout;
  opttime.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *) &opttime, sizeof opttime) < 0)
    throw ("setsockopt receive timeout");

  /* podpięcie się do grupy rozsyłania (ang. multicast) */
  struct ip_mreq mreq;
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (inet_aton(mcast_addr.c_str(), &mreq.imr_multiaddr) == 0)
    throw ("inet_aton");
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &mreq,
                 sizeof (ip_mreq)) < 0)
    throw ("setsockopt");

  client_address.sin_family = AF_INET;
  client_address.sin_addr.s_addr = htonl(INADDR_ANY);
  client_address.sin_port = htons(0);

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(cmd_port);
  if (inet_aton(mcast_addr.c_str(), &server_address.sin_addr) == 0)
    throw ("inet_aton");
  if (bind(sock, (struct sockaddr *) &client_address, sizeof(client_address)) < 0)
    throw "bind";
}

void ClientNode::Search(std::string filename) {

}
void ClientNode::Discover() {

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
//  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);

  if (vm.count("g")) {
    mcast_addr = vm["g"].as<std::string>();
  }
  if (vm.count("p")) {
    cmd_port = vm["p"].as<int>();
  }
  if (vm.count("o")) {
    path_to_folder = vm["o"].as<std::string>();
  }
  if (vm.count("t")) {
    timeout = vm["t"].as<int>();
  }
}

void ClientNode::StartWorking() {
  std::regex exit("exit");
  std::regex discover("discover");
  std::regex fetch("fetch (\\w*)\n");
  while (true) {
    std::smatch matched;

    std::string command;
    std::getline(std::cin, command);

    if (std::regex_search(command, matched, exit)) {
      break;
    } else if (std::regex_search(command, matched, discover)) {
      Discover();
    } else if (std::regex_search(command, matched, fetch)) {
      std::cout << matched[0];
    }

  }

}
