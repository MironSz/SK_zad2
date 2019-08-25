//
// Created by miron on 21.08.2019.
//

#define BUFFER_SIZE 1000
#define CMD_LENGTH 10
#include "SimpleCommand.h"
#include <memory>
#include <string>
#include <cstring>

SimpleCommand::SimpleCommand(std::string cmd,
                             uint64_t cmd_seq,
                             std::string data) : Command(cmd, cmd_seq, data, 0) {}

SimpleCommand::SimpleCommand(const SimpleCommand &simple_command) : Command(buffor_,
                                                                            GetSeq(),
                                                                            GetData(),
                                                                            0) {}

int SimpleCommand::DataBegin() {
  return 10 + sizeof(uint64_t);
}
