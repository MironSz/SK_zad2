//
// Created by miron on 21.08.2019.
//

#ifndef UNTITLED_MESSAGES_SIMPLECOMMAND_H_
#define UNTITLED_MESSAGES_SIMPLECOMMAND_H_
#include <memory>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Command.h"

class SimpleCommand : public Command {
 protected:
  int DataBegin() override;
 public:
  SimpleCommand(std::string buffor) : Command(buffor) {};
  SimpleCommand(const SimpleCommand &);
  SimpleCommand(std::string cmd, uint64_t cmd_seq, std::string data);
  SimpleCommand(int socket,
                int flags,
                struct sockadrr_in *src_addr,
                uint64_t seq_nr,
                std::string expected_command,
                socklen_t rcva_len = sizeof(sockaddr)) :
      Command(socket, flags, src_addr, seq_nr, expected_command, rcva_len) {};

};

#endif //UNTITLED_MESSAGES_SIMPLECOMMAND_H_
