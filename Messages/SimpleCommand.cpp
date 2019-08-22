//
// Created by miron on 21.08.2019.
//

#define BUFFER_SIZE 1000
#define CMD_LENGTH 10
#include "SimpleCommand.h"
#include <memory>
#include <string>
#include <cstring>
SimpleCommand::~SimpleCommand() {
  free(buffor);
}
SimpleCommand::SimpleCommand(std::string &cmd,
                             uint64_t cmd_seq,
                             std::string &data) {
  buffor = malloc(BUFFER_SIZE);
  memset(buffor, 0, BUFFER_SIZE);
  memcpy(buffor, cmd.c_str(), CMD_LENGTH);
  *(uint64_t *) ((char *) buffor + CMD_LENGTH) = htobe64(cmd_seq);
  memcpy((char *) buffor + CMD_LENGTH + sizeof(cmd_seq),
         data.c_str(),
         data.length());
  len = CMD_LENGTH + sizeof(cmd_seq) + data.length();
}
