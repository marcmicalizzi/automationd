/*
 * insteonplm.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_INSTEONPLM_H_
#define AUTOMATIOND_INSTEONPLM_H_

#include "insteon.h"

class Session;
class SerialSession;
class InsteonPLM: public Insteon {
  public:
    InsteonPLM(const std::string& tty, const std::string& devid);
    virtual ~InsteonPLM();
    Session* session() { return (Session*)session_; }
  private:
    SerialSession* session_;
};

#endif /* AUTOMATIOND_INSTEONPLM_H_ */
