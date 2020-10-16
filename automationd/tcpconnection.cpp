/*
 * TCPConnection.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#include "tcpconnection.h"
#include <iostream>
#include <thread>
#include <unistd.h>

#include "tcpsession.h"
#ifdef _WIN32
#include <winsock2.h>
#define lasterror WSAGetLastError()
#elif defined(__linux__)
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#endif

TCPConnection::TCPConnection(const std::string& host, const int& port) {
  host_ = host;
  port_ = port;

  std::cerr << "Connecting to server " << host << " on " << port << std::endl;
  socket_ = -1;
  session_ = 0;

  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ < 0)
      std::cerr << "Error opening socket" << std::endl;

  server_ = gethostbyname(host.c_str());
  memset((char *) &serv_addr_, '\0', sizeof(serv_addr_));
  serv_addr_.sin_family = AF_INET;
  memcpy((char *)&serv_addr_.sin_addr.s_addr, (char *)server_->h_addr, server_->h_length);
  serv_addr_.sin_port = htons(port);
}

TCPConnection::~TCPConnection() {
  delete session_;
}

bool TCPConnection::_connect() {
  if (!session_ || !session_->connected()) {
     if (session_) {
       delete session_;
     }
     if (connect(socket_, (struct sockaddr *) &serv_addr_, sizeof(serv_addr_)) < 0) {
         std::cerr << "Error connecting to server " << std::endl;
         return false;
     } else {
       session_ = new TcpSession(serv_addr_, socket_);
     }
   }
  return true;
}
