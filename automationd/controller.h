/*
 * controller.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_CONTROLLER_H_
#define AUTOMATIOND_CONTROLLER_H_

#include <string>

struct dev_status {
    dev_status() {
      level = -1;
      dbl = false;
      target = -1;
    }
    unsigned int target;
    unsigned int level;
    bool dbl;
};

class Controller {
  public:
    virtual ~Controller() { };
    virtual void add_device(const std::string& name, const std::string& devid) = 0;
    virtual void update_status() = 0;
    virtual void start() = 0;
    virtual bool get_status_update(std::pair<std::string, dev_status>& data) = 0;
};

#endif /* AUTOMATIOND_CONTROLLER_H_ */
