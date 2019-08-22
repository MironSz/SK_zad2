//
// Created by miron on 21.08.2019.
//

#ifndef SK2_NODE_LIB_H_
#define SK2_NODE_LIB_H_
#include <string>
#include <iostream>
//
//void logm(std::string message){
//  std::cout <<message<<"\n";
//}

void check_timeout(unsigned short value) {
  if (value <= 0 || value > 300) {
    throw "timeout of of bounds";
  }
}

void check_mc_addrs(std::string mc_addr){

}

void check_port(std::string mc_addr){

}

void check_fld(std::string){

}

#endif //SK2_NODE_LIB_H_
