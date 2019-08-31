//
// Created by miron on 22.08.2019.
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

int main(int argc, char **argv) {
  std::string mcast_addr_, path_to_folder_, type;
  int cmd_port_, timeout_;
  options_description desc{"Options"};
  desc.add_options()
      ("g", value<std::string>()->required(), "Adres rozglaszania ukierunkowanego")
      ("p", value<int>()->required(), "Port UDP")
      ("o", value<std::string>()->required(), "Path to files")
      ("t", value<int>()->default_value(5), "Timeout")
      ("n", value<std::string>(), "Node type");
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
  if (vm.count("n")) {
    type = vm["n"].as<std::string>();
  }
  if (vm.count("t")) {
    timeout_ = vm["t"].as<int>();
  }
  if(type == "server"){
    sockaddr_in dest_addres;
    int sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
    dest_addres.sin_port = htobe64(40002);
    dest_addres.sin_family = AF_INET; // IPv4
    dest_addres.sin_addr.s_addr = htobe64(INADDR_ANY);
    socklen_t len  = sizeof(sockaddr_in);
    if (bind(sock, (sockaddr *)&dest_addres, sizeof dest_addres)){
      std :: cout << "error bind 1";
      return 1;
    }
  }



}
