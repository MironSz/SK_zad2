//
// Created by miron on 21.08.2019.
//
#include <stdio.h>
#include <cstring>
#include "ComplexCommand.h"
#define BUFFER_SIZE 1000
#define CMD_LENGTH 10

ComplexCommand::ComplexCommand(std::string &cmd,
                               uint64_t cmd_seq,
                               uint64_t param,
                               std::string &data) : Command(cmd, cmd_seq, data, (int)sizeof(uint64_t)) {
  memcpy((char *)SeqBegin()+sizeof(uint64_t), (void *) &param, sizeof(param));
}

int ComplexCommand::DataBegin() {
  return 10 + 2 * sizeof(uint64_t);
}
uint64_t ComplexCommand::GetParam() {
  return *(uint64_t  *)((char *)SeqBegin()+ sizeof(uint64_t));
}
