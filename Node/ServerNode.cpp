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
  if (vm.count("o")) {
    path_to_folder_ = vm["o"].as<std::string>();
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
  ip=std::string(IPbuffer);
  std::cout <<ip<<"\n";
}
void ServerNode::StartWorking() {
  log_message("Started working");

  while (true) {
    Command *request = Command::ReadCommand(multicast_socket_,
                                            0,
                                            reinterpret_cast< sockadrr_in *>(&client_address_),
                                            0,
                                            sizeof(sockaddr_in));
    log_message("Received request " + request->GetCommand());

    if (request->GetCommand() == "HELLO") {
      Discover(request);
    } else if (request->GetCommand() == "SERVER_EMERGENCY_SHUTDOWN_PROTOCOL_OVER_9000") {
      break;
    }

  }

}
void ServerNode::IndexFiles() {
  DIR *dir;
  dirent *ent;
  if ((dir = opendir(path_to_folder_.c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr) {
      if (ent->d_type == DT_REG) {
        std::string filename(ent->d_name);
        files.push_back(filename);
      }
    }
    closedir(dir);
  } else {
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




