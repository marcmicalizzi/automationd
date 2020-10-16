/*
 * insteon.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#include "insteonhub.h"
#include "tcpsession.h"

InsteonHub::InsteonHub(const std::string& host, const std::string& devid) : Insteon(devid), TCPConnection(host, 9761) {
  _connect();
  delay_ = 110000;
}

InsteonHub::~InsteonHub() {

}
