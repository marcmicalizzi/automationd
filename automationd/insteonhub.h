/*
 * insteon.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_INSTEONHUB_H_
#define AUTOMATIOND_INSTEONHUB_H_

#include <string>
#include "tcpconnection.h"
#include "insteon.h"

class InsteonHub : public Insteon, public TCPConnection {
  public:
    InsteonHub(const std::string& host, const std::string& devid);
    virtual ~InsteonHub();
    Session* session() { return (Session*)session_; }
};

#endif /* AUTOMATIOND_INSTEONHUB_H_ */
