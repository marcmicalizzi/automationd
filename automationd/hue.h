/*
 * hue.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_HUE_H_
#define AUTOMATIOND_HUE_H_

#include <string>
#include <thread>
#include <atomic>
#include <map>
#include "msgq.h"
#include "responder.h"

class Sun;
class PhilipsHue : public Responder {
  struct cmd_t {
    std::string name;
    int level;
    std::string args;
  };
  public:
    PhilipsHue(const std::string& host, const std::string& apikey);
    virtual ~PhilipsHue();
    void set_location(int latitude, int longitude);
    void add_light(const std::string& name, const std::string& req);
    void set_level(const std::string& name, int level, const std::string& args);
    void notify(const std::string& name, const std::string& args);
    void set_sun_settings(const std::string& name, double sun_set_end, double sun_set_begin, double day_temp, double night_temp);
    void start();
  private:
    Sun* sun_;
    std::string host_;
    std::string apikey_;
    std::thread* thread_;
    std::atomic<bool> exit_;
    msgq_t<cmd_t> cmd_q_;
    std::map<std::string, std::string> lights_;
    std::map<std::string, sun_setting_t> sun_setting_;
    void run();
    void do_put_req(const std::string& requrl, const std::string& data);
};

#endif /* AUTOMATIOND_HUE_H_ */
