/*
 * engine.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#include "engine.h"
#include <unistd.h>
#include "controller.h"
#include <iostream>
#include "responder.h"
#include <chrono>
#include <thread>

AutomationEngine::AutomationEngine() {
  exit_ = false;
}

AutomationEngine::~AutomationEngine() {
  exit_ = true;
}

void AutomationEngine::add_controller(Controller* c) {
  controllers_.push_back(c);
}

void AutomationEngine::add_binding(const std::string& device_name, int param, Responder* target, ActionType action, const std::string& args) {
  std::pair<std::string, int> tgt(device_name, param);
  auto it = bindings_.find(tgt);
  if (it == bindings_.end()) {
    bindings_[tgt] = std::list<binding_t>();
  }
  binding_t b;
  b.device = target;
  b.args = args;
  b.action = action;
  bindings_[tgt].push_back(b);
}

void AutomationEngine::add_secondary_binding(const std::string& device_name, int param, const std::string& target_name, bool primary_affects, Responder* target, ActionType action, const std::string& args) {
  std::pair<std::string, int> tgt(device_name, param);
  auto it = secondary_bindings_.find(tgt);
  if (it == secondary_bindings_.end()) {
    secondary_bindings_[tgt] = std::list<std::pair<std::string, binding_t> >();
  }
  binding_t b;
  b.device = target;
  b.args = args;
  b.action = action;
  b.pri_affects = primary_affects;
  secondary_bindings_[tgt].push_back(std::pair<std::string, binding_t>(target_name, b));
}

void AutomationEngine::run() {
  for (size_t i = 0; i < controllers_.size(); i++) {
    controllers_[i]->start();
  }
  while (!exit_) {
    for (size_t i = 0; i < controllers_.size(); i++) {
      std::pair<std::string, dev_status> item;
      if (controllers_[i]->get_status_update(item)) {
        std::pair<std::string, int> tgt(item.first, item.second.target);
		std::cerr << "AutomationEngine: Got status update " << item.first << ":" << item.second.target << std::endl;
        auto kt = last_value_.find(tgt);
        if (kt != last_value_.end() && kt->second.level == item.second.level) {
          std::cerr << "AutomationEngine: Duplicate level value, checking secondary bindings" << std::endl;
          auto lt = secondary_bindings_.find(tgt);
		  if (lt == secondary_bindings_.end()) {
            std::cerr << "AutomationEngine: No secondary binding for group " << item.second.target << " checking for default..." << std::endl;
		    lt = secondary_bindings_.find({item.first, -1});
		  }
          if (lt != secondary_bindings_.end()) {
            std::cerr << "AutomationEngine: Using secondary binding" << std::endl;
            if (item.second.dbl) {
              std::pair<std::string, binding_t> binding = lt->second.front();
              lt->second.pop_front();
              lt->second.push_back(binding);
              lt->second.front().second.device->notify(lt->second.front().first, lt->second.front().second.args);
            } else {
              int level = -1;
              auto mt = secondary_last_value_.find(lt->second.front());
              if (mt != secondary_last_value_.end()) {
                level = mt->second;
              }
              switch (lt->second.front().second.action) {
                case LevelControl:
                  if (level > 0) {
                    level = 0;
                  } else {
                    level = 0xff;
                  }
                  lt->second.front().second.device->set_level(lt->second.front().first, level, lt->second.front().second.args);
                  break;
              }
              secondary_last_value_[lt->second.front()] = level;
            }
          }
          continue;
        }
        last_value_[tgt] = item.second;

        auto it = bindings_.find(std::pair<std::string, int>(item.first, item.second.target));
		if (it == bindings_.end()) {
			it = bindings_.find(std::pair<std::string, int>(item.first, -1));
			std::cerr << "AutomationEngine: No binding for group " << item.second.target << " checking for default..." << std::endl;
		}
        if (it != bindings_.end()) {
	      std::cerr << "AutomationEngine: Executing primary binding" << std::endl;
          for (auto jt = it->second.begin(); jt != it->second.end(); jt++) {
            switch (jt->action) {
              case LevelControl:
                std::cerr << "AutomationEngine: Executing level control" << std::endl;
                jt->device->set_level(item.first, item.second.level, jt->args);
                break;
              default:
                std::cerr << "AutomationEngine: Action type " << jt->action << " not handled; discarding" << std::endl;
            }
            auto kt = secondary_bindings_.find(std::pair<std::string, int>(item.first, item.second.target));
            if (kt != secondary_bindings_.end()) {
              for (auto lt = kt->second.begin(); lt != kt->second.end(); lt++) {
                if (lt->second.pri_affects && lt->second.action == jt->action) {
                  secondary_last_value_[*lt] = item.second.level;
                }
              }
            }
          }
        } else {
			std::cerr << "AutomationEngine: Binding not found" << std::endl;
		}
      }
    }
  }
}
