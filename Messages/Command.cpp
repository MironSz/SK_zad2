//
// Created by miron on 23.08.2019.
//

#include <cstring>
#include "Command.h"
#include "../Lib.h"
#include "SimpleCommand.h"
#include "ComplexCommand.h"
int Command::GetLen() {
  return buffor_.length();
}
std::string Command::GetData() {
  return buffor_.substr(DataBegin());
}
std::string Command::GetCommand() {
  int first_zero = buffor_.find_first_of('\000');
  return buffor_.substr(0, first_zero);
}

Command::Command(std::string cmd, uint64_t cmd_seq, std::string data, int param_size) {
//  Set command
  buffor_ = cmd;
//  Fill with zeroes
  buffor_.resize(CMD_LENGTH + sizeof(uint64_t) + param_size + data.length(), 0);
//  Set cmd_seq
  memcpy(SeqBegin(), (void *) &cmd_seq, sizeof(cmd_seq));
//  Set data
  memcpy((char *) SeqBegin() + param_size, data.data(), data.size());
}
uint64_t Command::GetSeq() {
  return *(uint64_t *) SeqBegin();
}
void *Command::SeqBegin() {
  return (void *) (buffor_.data() + CMD_LENGTH);
}
void *Command::CmdBegin() {
  return (void *) (buffor_.data());
}

int Command::SendTo(int sock, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
  if (sendto(sock, buffor_.data(), buffor_.length(), 0, dest_addr, sizeof(*dest_addr))
      != buffor_.length()) {
//  TODO: Unable to send message
  }
  return buffor_.length();
}

Command::Command(int socket, int flags, struct sockadrr_in *src_addr, uint64_t seq_nr) {
  log_message("Command: Waiting for message");
  char inner_buffer[BUFFER_SIZE];
  int rcva_len = (socklen_t) sizeof(src_addr);

  int bytes_read = recvfrom(socket, inner_buffer, BUFFER_SIZE, 0, (struct sockaddr *) &src_addr,
                            reinterpret_cast<socklen_t *>(&rcva_len));
  log_message("Command: Received bytes "+std::to_string(bytes_read));

  while (bytes_read > 0 && *(uint64_t *) (inner_buffer + CMD_LENGTH) != seq_nr
      && seq_nr != 0) {
    bytes_read = recvfrom(socket, inner_buffer, BUFFER_SIZE, 0, (struct sockaddr *) &src_addr,
                          reinterpret_cast<socklen_t *>(&rcva_len));
  }
  if (bytes_read > 0) {
    buffor_.resize(bytes_read);
    memcpy((void *) buffor_.data(), inner_buffer, bytes_read);
  } else {
    buffor_ = std::string();
  }

}
Command *Command::ReadCommand(int socket,
                              int flags,
                              struct sockadrr_in *src_addr,
                              uint64_t seq_nr) {
  Command *result = new SimpleCommand(socket, flags, src_addr, seq_nr);

  if (result->GetCommand() == "GOOD_DAY" ||
      result->GetCommand() == "ADD" ||
      result->GetCommand() == "CONNECT_ME" ||
      result->GetCommand() == "CAN_ADD" ) {
    result = new ComplexCommand(result->buffor_);
  }

  return result;
}
