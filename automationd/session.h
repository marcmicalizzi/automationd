/*
 * session.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_SESSION_H_
#define AUTOMATIOND_SESSION_H_

#include <cstdlib>

class Session {
  public:
    virtual ~Session() { }
    virtual void send_packet(unsigned char* data, size_t len) = 0;
    virtual unsigned char* recv_packet(size_t& size) = 0;

    virtual bool connected() const = 0;
};

#endif /* AUTOMATIOND_SESSION_H_ */
