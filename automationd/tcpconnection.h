/*
 * TCPConnection.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_TCPCONNECTION_H_
#define AUTOMATIOND_TCPCONNECTION_H_

#include <string>
#ifdef _WIN32
#include <winsock2.h>
#elif defined(__linux__)
#include <netinet/in.h>
#endif

class TcpSession;
struct hostent;
class TCPConnection {
  public:
    TCPConnection(const std::string& host, const int& port);
    virtual ~TCPConnection();
  protected:
    TcpSession* session_;
    bool _connect();
  private:
    std::string host_;
    int port_;
#ifdef _WIN32
    long long socket_;
#elif defined(__linux__)
    int socket_;
#endif
    struct sockaddr_in serv_addr_;
    struct hostent* server_;
};

#endif /* AUTOMATIOND_TCPCONNECTION_H_ */
