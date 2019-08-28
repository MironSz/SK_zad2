//
// Created by miron on 21.08.2019.
//

#ifndef UNTITLED_MESSAGES_COMPLEXCOMMAND_H_
#define UNTITLED_MESSAGES_COMPLEXCOMMAND_H_
#include <memory>
#include <string>
#include "Command.h"

class ComplexCommand : public Command {
 public:
  ComplexCommand(std::string buffor) : Command(buffor) {};
  ComplexCommand(int socket,
                 int flags,
                 sockadrr_in *src_addr,
                 uint64_t seq_nr,
                 std::string expected_command,
                 socklen_t rcva_len = sizeof(sockaddr_in)) :
      Command(socket, flags, src_addr, seq_nr,expected_command, rcva_len) {}

  ComplexCommand(std::string &cmd,
                 uint64_t cmd_seq,
                 uint64_t param,
                 std::string &data);
  ComplexCommand(const ComplexCommand &) = default;
  uint64_t GetParam();
 private:

  int DataBegin() override;
};

#endif //UNTITLED_MESSAGES_COMPLEXCOMMAND_H_
