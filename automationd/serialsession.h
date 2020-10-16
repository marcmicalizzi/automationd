/*
 * serialsession.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_SERIALSESSION_H_
#define AUTOMATIOND_SERIALSESSION_H_

#include "session.h"
#include <string>

class SerialSession_impl;
class SerialSession: public Session {
  public:
    SerialSession(const std::string& dev);
    virtual ~SerialSession();
    void send_packet(unsigned char* data, size_t len);
    unsigned char* recv_packet(size_t& size);

    bool connected() const;
  private:
    SerialSession_impl* pimpl_;
};

#endif /* AUTOMATIOND_SERIALSESSION_H_ */
