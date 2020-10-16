/*
 * insteon.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_INSTEON_H_
#define AUTOMATIOND_INSTEON_H_

#include <string>
#include <map>
#include <list>
#include <thread>
#include <atomic>
#include <mutex>
#include "msgq.h"
#include "controller.h"
#include "responder.h"
#include <sys/types.h>

enum InsteonDevice {
  InsteonDimmer = 0,
  InsteonKeypad = 1
};

class Session;
class Insteon : public Controller, public Responder {
  enum DimmerType {
    DimmerNone = 0,
    DimmerDimming = 1,
    DimmerBrightening = 2
  };
  public:
    Insteon(const std::string& devid);
    virtual ~Insteon();
    void add_device(const std::string& name, const std::string& devid);
    void bind_device(const std::string& dev1, const std::string& dev2);
    void update_status();
    void start();
    bool get_status_update(std::pair<std::string, dev_status>& data);
    void set_level(const std::string& name, int level, const std::string& args);
    void notify(const std::string& name, const std::string& args) { }
    virtual Session* session() = 0;
  protected:
    useconds_t delay_;
  private:
    struct message_t {
      std::string src;
      std::string dst;
      unsigned char hops = 0xff;
      unsigned char cmd1 = 0;
	  unsigned char cmd2 = 0;
	};
	message_t last_acted_message_;
    std::string dev_id_;
    std::map<std::string, std::string> devices_;
    std::map<std::string, std::string> devices_byid_;
    std::map<std::string, dev_status> status_;
    std::map<std::string, std::string>::iterator cur_pos_;
    std::map<std::string, std::atomic<int> > dimming_status_;
    std::list<unsigned char> buffer_;
    msgq_t<std::pair<std::string, dev_status> > status_q_;
    std::atomic<bool> exit_;
    std::mutex status_lock_;
    std::thread* thread_;
    std::thread* dimming_thread_;
    unsigned char* last_command_data_;
    size_t last_command_size_;
    char cur_message_[2];
    bool wait_;

    void run();
    void run_dimmer();
    bool buffer(unsigned char** recv_data, size_t& recv_size, size_t expected);
};

#endif /* AUTOMATIOND_INSTEON_H_ */
