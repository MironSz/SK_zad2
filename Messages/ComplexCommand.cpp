//
// Created by miron on 21.08.2019.
//
#include <stdio.h>
#include <cstring>
#include "ComplexCommand.h"
#define BUFFER_SIZE 1000
#define CMD_LENGTH 10

ComplexCommand::~ComplexCommand() {
  free(buffor);
}
ComplexCommand::ComplexCommand(std::string &cmd,
                               uint64_t cmd_seq,
                               uint64_t param,
                               std::string &data) {
  buffor = malloc(BUFFER_SIZE);
  memset(buffor, 0, BUFFER_SIZE);
  memcpy(buffor, cmd.c_str(), CMD_LENGTH);
  *(uint64_t *) ((char *) buffor + CMD_LENGTH) = htobe64(cmd_seq);
  *(uint64_t *) ((char *) buffor + sizeof(cmd_seq) + CMD_LENGTH) = htobe64(param);
  memcpy((char *) buffor + CMD_LENGTH + sizeof(cmd_seq) + sizeof(param),
         data.c_str(),
         data.length());
  len = CMD_LENGTH + sizeof(cmd_seq) + sizeof(param) + data.length();
}
