/*
 * engine.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_ENGINE_H_
#define AUTOMATIOND_ENGINE_H_

#include <atomic>
#include <vector>
#include <list>
#include <string>
#include <map>
#include "controller.h"

class Responder;
enum ActionType {
  LevelControl = 0
};

class AutomationEngine {
  struct binding_t {
      binding_t() {
        device = 0;
        action = (ActionType)-1;
        pri_affects = false;
      }
      Responder* device;
      std::string args;
      ActionType action;
      bool pri_affects;
      bool operator==(const binding_t& rhs) const {
        return rhs.device == device &&
               rhs.args == args &&
               rhs.action == action;
      }
      bool operator>(const binding_t& rhs) const {
        return (unsigned long long)rhs.device < (unsigned long long)device &&
               rhs.args < args &&
               rhs.action < action;
      }
      bool operator<(const binding_t& rhs) const {
        return (unsigned long long)rhs.device > (unsigned long long)device &&
               rhs.args > args &&
               rhs.action > action;
      }
  };
  public:
    AutomationEngine();
    virtual ~AutomationEngine();

    void add_controller(Controller* c);
    void add_binding(const std::string& device_name, int param, Responder* target, ActionType action, const std::string& args = "");
    void add_secondary_binding(const std::string& device_name, int param, const std::string& target_name, bool primary_affects, Responder* target, ActionType action, const std::string& args = "");
    void run();
  private:
    std::atomic<bool> exit_;
    std::vector<Controller*> controllers_;
    std::map<std::pair<std::string, int>, std::list<binding_t> > bindings_;
    std::map<std::pair<std::string, int>, dev_status> last_value_;
    std::map<std::pair<std::string, int>, std::list<std::pair<std::string, binding_t> > > secondary_bindings_;
    std::map<std::pair<std::string, binding_t>, unsigned int> secondary_last_value_;
};

#endif /* AUTOMATIOND_ENGINE_H_ */
