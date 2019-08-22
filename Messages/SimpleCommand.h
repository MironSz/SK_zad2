//
// Created by miron on 21.08.2019.
//

#ifndef UNTITLED_MESSAGES_SIMPLECOMMAND_H_
#define UNTITLED_MESSAGES_SIMPLECOMMAND_H_
#include <memory>
#include <string>

class SimpleCommand {
 public:
  void * buffor;
  int len;
  SimpleCommand(std::string &cmd, uint64_t cmd_seq, std::string &data);
  virtual ~SimpleCommand();
};

#endif //UNTITLED_MESSAGES_SIMPLECOMMAND_H_
