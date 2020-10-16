/*
 * session.h
 *
 *  Created on: Sep 17, 2014
 *      Author: marc.micalizzi
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <cstdlib>
#include "session.h"

struct sockaddr_in;
class TcpSession_impl;
class TcpSession : public Session {
  public:
    TcpSession(const struct sockaddr_in& sock, int fd);
    virtual ~TcpSession();

    void send_packet(unsigned char* data, size_t len);
    unsigned char* recv_packet(size_t& size);

    bool connected() const;
  private:
    TcpSession_impl* pimpl_;
};

#endif /* SESSION_H_ */
