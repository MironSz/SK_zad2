//
// Created by miron on 21.08.2019.
//


#include "Node/ClientNode.h"
int main( int argc,char ** argv){
  ClientNode client_node(argv,argc);
  client_node.StartWorking();
}
