//
// Created by miron on 28.08.2019.
//

#ifndef SK2_NODE_NODE_H_
#define SK2_NODE_NODE_H_

class Node {
 protected:
  static void SendFile(int sock, sockaddr_in client_addr, int multicast_socket, std::string path_to_input_dir);

};

#endif //SK2_NODE_NODE_H_
