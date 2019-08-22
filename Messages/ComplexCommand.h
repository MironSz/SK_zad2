//
// Created by miron on 21.08.2019.
//

#ifndef UNTITLED_MESSAGES_COMPLEXCOMMAND_H_
#define UNTITLED_MESSAGES_COMPLEXCOMMAND_H_
#include <memory>
#include <string>


class ComplexCommand {
 public:
  void * buffor;
  int len;
  ComplexCommand(std::string &cmd, uint64_t cmd_seq,uint64_t param, std::string &data);
  virtual ~ComplexCommand();
};

#endif //UNTITLED_MESSAGES_COMPLEXCOMMAND_H_
