//
// Created by miron on 21.08.2019.
//

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "Node/ServerNode.h"

ServerNode *serverNodePointer;
void my_handler(int s) {
  serverNodePointer->~ServerNode();
  exit(1);
}
int main(int argc, char **argv) {
  ServerNode server_node(argv, argc);
  serverNodePointer = &server_node;
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
  server_node.StartWorking();
}
