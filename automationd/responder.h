/*
 * target.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_RESPONDER_H_
#define AUTOMATIOND_RESPONDER_H_

#include <string>

struct sun_setting_t {
    double min_val;
    double max_val;
    double min_cond;
    double max_cond;
    double last_val;
};

class Responder {
  public:
    virtual ~Responder() { };
    virtual void set_level(const std::string& name, int level, const std::string& args) = 0;
    virtual void notify(const std::string& name, const std::string& args) = 0;
    virtual void start() = 0;
};

#endif /* AUTOMATIOND_RESPONDER_H_ */
