/*
 * hue.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#include "hue.h"
#include "sun.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <unistd.h>

PhilipsHue::PhilipsHue(const std::string& host, const std::string& apikey) {
  sun_ = 0;
  host_ = host;
  apikey_ = apikey;
  thread_ = 0;
  exit_ = false;
  start();
}

PhilipsHue::~PhilipsHue() {
  exit_ = true;
  if (thread_) {
    thread_->join();
    delete thread_;
  }
}

void PhilipsHue::set_location(int latitude, int longitude) {
  if (sun_) delete sun_;
  sun_ = new Sun(latitude, longitude);
}

void PhilipsHue::add_light(const std::string& name, const std::string& req) {
  lights_[name] = req;
}

void PhilipsHue::set_level(const std::string& name, int level, const std::string& args) {
  cmd_t c;
  c.name = name;
  c.level = level;
  c.args = args;
  cmd_q_.send(c);
}

void PhilipsHue::notify(const std::string& name, const std::string& args) {
  cmd_t c;
  c.name = name;
  c.level = -1;
  c.args = args;
  cmd_q_.send(c);
}

void PhilipsHue::set_sun_settings(const std::string& name, double sun_set_end, double sun_set_begin, double day_temp, double night_temp) {
  sun_setting_t s;
  s.min_cond = sun_set_end;
  s.max_cond = sun_set_begin;
  s.min_val = night_temp;
  s.max_val = day_temp;
  s.last_val = -1;
  sun_setting_[name] = s;
}

void PhilipsHue::start() {
  thread_ = new std::thread(&PhilipsHue::run, std::ref(*this));
}

void PhilipsHue::do_put_req(const std::string& requrl, const std::string& data) {
  CURL* curl = curl_easy_init();
  std::string url = "http://" + host_ + "/api/" + apikey_ + "/" + requrl;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
  std::cerr << "sending: " << data << " to " << requrl << std::endl;
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  usleep(250000);
}

void get_xy_k(double k, double& x, double& y) {
  x = -2.0064*(1000000000/pow(k, 3.0))+1.9018*(1000000/pow(k, 2.0))+0.24748*(1000/k)+0.23704;
  y = -3*pow(x, 2) + 2.87*x - 0.275;
}

void PhilipsHue::run() {
  while (!exit_) {
    cmd_t cmd;
    if (cmd_q_.get(cmd, std::chrono::microseconds(10000000))) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": AutomationEngine wants me to set " << cmd.name << " to " << cmd.level << std::endl;

      std::stringstream ss;
      ss << "{";
      if (cmd.level == 0) {
        ss << "\"on\":false";
      } else if (cmd.level > 0) {
        ss << "\"on\":true, \"bri\":" << (int)((double)cmd.level);
        if (sun_) {
          auto it = sun_setting_.find(cmd.name);
          if (it != sun_setting_.end()) {
            int tmp = 0;
            double s = sun_->get_elevation();
            if (s > it->second.max_cond) {
              tmp = it->second.max_val;
            } else if (s < it->second.min_cond) {
              tmp = it->second.min_val;
            } else {
              tmp = it->second.min_val + ((it->second.max_val - it->second.min_val) * ((s - it->second.min_cond) / (it->second.max_cond - it->second.min_cond)));
            }
            if (cmd.level > 0xf0) {
              it->second.last_val = tmp;
              double x, y;
              get_xy_k(tmp, x, y);
              ss << ", \"ct\":" << round(1000000/tmp) << "";
            }
          }
        }
      } else if (cmd.level < 0) {
        ss << "\"alert\":\"select\"";
      }
      ss << "}";
      auto it = lights_.find(cmd.name);
      if (it != lights_.end()) {
        do_put_req(it->second, ss.str());
      }

    }
    if (sun_) {
      for (auto it = sun_setting_.begin(); it != sun_setting_.end(); it++) {
        int tmp = 0;
        double s = sun_->get_elevation();
        double range = it->second.max_val - it->second.min_val;
        if (s > it->second.max_cond) {
          tmp = it->second.max_val;
        } else if (s < it->second.min_cond) {
          tmp = it->second.min_val;
        } else {
          tmp = it->second.min_val + ((range) * ((s - it->second.min_cond) / (it->second.max_cond - it->second.min_cond)));
        }
        if ((abs(tmp - it->second.last_val) > 100)) {
          std::stringstream ss;
          double x, y;
          get_xy_k(tmp, x, y);
          ss << "{\"ct\":" << round(1000000/tmp) << ", \"transitiontime\":200}";
          auto jt = lights_.find(it->first);
          if (jt != lights_.end()) {
            do_put_req(jt->second, ss.str());
          }
          it->second.last_val = tmp;
        }
      }
    }
  }
}
