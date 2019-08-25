//
// Created by miron on 21.08.2019.
//


#include "Node/ServerNode.h"
int main( int argc,char ** argv){
  ServerNode server_node(argv,argc);
  server_node.StartWorking();
}
