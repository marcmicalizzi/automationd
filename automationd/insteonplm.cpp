/*
 * insteonplm.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#include "insteonplm.h"
#include "serialsession.h"

InsteonPLM::InsteonPLM(const std::string& tty, const std::string& devid) : Insteon(devid) {
  session_ = new SerialSession(tty);
}

InsteonPLM::~InsteonPLM() {
  delete session_;
}

