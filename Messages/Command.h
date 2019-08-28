//
// Created by miron on 23.08.2019.
//

#ifndef SK2_MESSAGES_COMMAND_H_
#define SK2_MESSAGES_COMMAND_H_
#include <memory>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

class Command {
 protected:
  std::string buffor_;
  virtual int DataBegin() = 0;
  void *SeqBegin();
  void *CmdBegin();

 public:
  std::string &GetBuffer();
  Command(std::string buffor) : buffor_(buffor) {};
  static Command *ReadCommand(int socket,
                              int flags,
                              struct sockadrr_in *src_addr,
                              uint64_t seq_nr,
                              std::string expected_command,
                              socklen_t rcva_len);
  Command(int socket,
          int flags,
          struct sockadrr_in *src_addr,
          uint64_t seq_nr,
          std::string expected_command,
          socklen_t rcva_len = sizeof(sockaddr_in));
  Command(std::string cmd, uint64_t cmd_seq, std::string data, int data_begin);
  std::string GetData();
  std::string GetCommand();
  int GetLen();
  uint64_t GetSeq();

  int SendTo(int sock, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
};

#endif //SK2_MESSAGES_COMMAND_H_
