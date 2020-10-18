/*
 * insteon.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#include "insteon.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <locale>
#include <boost/algorithm/string.hpp>
#include <sys/time.h>
#include "session.h"

Insteon::Insteon(const std::string& devid) {
  dev_id_ = devid;
  boost::to_upper(dev_id_);
  cur_pos_ = devices_.end();
  exit_ = false;
  thread_ = 0;
  dimming_thread_ = 0;
  last_command_data_ = 0;
  last_command_size_ = 0;
  wait_ = false;
  cur_message_[0] = 0x00;
  cur_message_[1] = 0x00;
  delay_ = 0;
}

Insteon::~Insteon() {
  exit_ = true;
}

void Insteon::add_device(const std::string& name, const std::string& devid) {
  std::string _devid = devid;
  boost::to_upper(_devid);
  devices_[name] = _devid;
  devices_byid_[devid] = name;
  dimming_status_[name] = DimmerNone;
}

void Insteon::bind_device(const std::string& dev1, const std::string& dev2) {
  std::string _dev1 = dev1;
  std::string _dev2 = dev2;
  auto it = devices_.find(_dev1);
  if (it != devices_.end()) {
    devices_byid_[it->second] = _dev2;
    std::cerr << "Insteon: Binding " << _dev2 << " to " << it->second << std::endl;
  }
}

void Insteon::update_status() {
  return;
  if (cur_pos_ == devices_.end()) {
    cur_pos_ = devices_.begin();
    if (cur_pos_ == devices_.end()) return;
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);

  std::unique_lock<std::mutex> lock(status_lock_);
  auto dt = status_.find(cur_pos_->first);
  if (dt != status_.end()) return;
  lock.unlock();

  std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": Insteon: Checking status of " << cur_pos_->first << std::endl;

  unsigned char req[8];
  req[0] = 0x02;//direct
  req[1] = 0x62;//command
  unsigned long device = std::stoul(cur_pos_->second.c_str(), 0, 16);
  req[2] = (device & 0xff0000) >> 16;
  req[3] = (device & 0x00ff00) >> 8;
  req[4] = device & 0x0000ff;
  req[5] = 0x0f;//sd flag?
  req[6] = 0x19;//status command
  req[7] = 0x02;//level?

  last_command_data_ = new unsigned char[8];
  memcpy(last_command_data_, req, 8);
  last_command_size_ = 8;

  session()->send_packet(req, 8);
  cur_pos_++;
}

void Insteon::start() {
  if (!thread_) {
    thread_ = new std::thread(&Insteon::run, std::ref(*this));
  }
  if (!dimming_thread_) {
    dimming_thread_ = new std::thread(&Insteon::run_dimmer, std::ref(*this));
  }
}

bool Insteon::get_status_update(std::pair<std::string, dev_status>& data) {
  return status_q_.get(data, std::chrono::microseconds(10000));
}

void Insteon::set_level(const std::string& name, int level, const std::string& args) {

}

bool Insteon::buffer(unsigned char** recv_data, size_t& recv_size, size_t expected) {
  if (!recv_data || (recv_size && !*recv_data)) return false;
  for (size_t i = 0; i < recv_size; i++) {
    buffer_.push_back((*recv_data)[i]);
  }
  delete[] *recv_data;
  if (buffer_.size() >= expected) {
    unsigned char* data = new unsigned char[expected];
    size_t i = 0;
    for (auto it = buffer_.begin(); it != buffer_.end() && i < expected;) {
      data[i] = (*it);
      auto jt = it;
      it++;
      buffer_.erase(jt);
      i++;
    }
    *recv_data = data;
    recv_size = expected;
    std::cerr << "Insteon Recv: ";
    for (size_t i = 0; i < recv_size; i++) {
      std::cerr << std::setw(2) << std::setfill('0') << std::hex << (((unsigned short)(*recv_data)[i]) & 0xff);
    }
    std::cerr << std::endl;
    return true;
  } else {
    *recv_data = 0;
    recv_size = 0;
    return false;
  }
}

void Insteon::run() {
  size_t recv_size = 0;
  unsigned char* recv_data = 0;
  while (!exit_) {
    recv_data = session()->recv_packet(recv_size);
    bool need_delay = false;
    if (recv_data) {
      if (last_command_data_) {//waiting for response to command
        if (recv_data[0] == 0x15) {
          struct timeval tv;
          gettimeofday(&tv, NULL);
          std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": Insteon: (Waiting for response) Got NAK, retrying" << std::endl;
          delete[] recv_data;
          session()->send_packet(last_command_data_, last_command_size_);
          buffer_.clear();
          continue;
        }
        size_t size = (recv_size < last_command_size_ ? recv_size : last_command_size_);
        if (recv_size < 9 || !buffer_.empty()) {
          if (!buffer(&recv_data, recv_size, 9)) {
            continue;
          }
        }
        bool cmp = true;
        for (size_t i = 0; i < size; i++) {
          if (last_command_data_[i] != recv_data[i]) {
            cmp = false;
            break;
          }
        }
        if (cmp) {
          struct timeval tv;
          gettimeofday(&tv, NULL);
          std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": Insteon: Got ACK" << std::endl;
          wait_ = true;
          delete[] last_command_data_;
          last_command_data_ = 0;
          delete[] recv_data;
		  recv_data = 0;
		  recv_size = 0;
          continue;
        } else {
          std::cerr << "Insteon: (Waiting for response) Got different data, passing to message parsing" << std::endl;
        }
      }
      if (cur_message_[0] == 0x00 && cur_message_[1] == 0x00) {
        if (recv_data && recv_data[0] == 0x15) {
          struct timeval tv;
          gettimeofday(&tv, NULL);
          std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": Insteon: (Waiting for message) Got NAK, flushing buffer" << std::endl;
          delete[] recv_data;
		  recv_data = 0;
		  recv_size = 0;
          buffer_.clear();
          continue;
        }
        if (!buffer(&recv_data, recv_size, 2)) {
          std::cerr << "Insteon: New message buffer failed, waiting for more data" << std::endl;
          continue;
        } else {
          if (recv_data[0] == 0x02) {
            memcpy(cur_message_, recv_data, 2);
			std::cerr << "Insteon: New message set to " << std::setw(2) << std::setfill('0') << std::hex << (unsigned short) cur_message_[0] << (unsigned short) cur_message_[1] << std::endl;
		  } else if (recv_data[1] == 0x02) {
			std::cerr << "Insteon: New message first byte invalid. Discarding." << std::endl;
			buffer_.push_front(recv_data[1]);
          } else {
            std::cerr << "Insteon: New message unexpected character" << std::endl;
		  }
          delete[] recv_data;
          recv_data = 0;
          recv_size = 0;
        }
      }
      if (cur_message_[0] == 0x02) {
        if (cur_message_[1] == 0x50) {//Status Standard command
          if (!buffer(&recv_data, recv_size, 9)) {
            continue;
          }
          struct timeval tv;
          gettimeofday(&tv, NULL);
          std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": Insteon: Got Standard Command" << std::endl;
          std::stringstream ss;
          ss << std::setw(2) << std::setfill('0') << std::hex << ((unsigned int)recv_data[0] & 0xff) << std::setw(2) << std::setfill('0') << ((unsigned int)recv_data[1] & 0xff) << std::setw(2) << std::setfill('0') << ((unsigned int)recv_data[2] & 0xff);
          std::string device = ss.str();
          boost::to_upper(device);
          gettimeofday(&tv, NULL);
          std::cerr << std::fixed << std::setprecision(1) << ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000)) << ": Insteon: Source device: " << device << std::endl;
          auto it = devices_byid_.find(device);
          if (it != devices_byid_.end()) {
            ss << "Insteon: Target location: " << it->second << std::endl;
            ss.str("");
            ss << std::setw(2) << std::setfill('0') << std::hex << ((unsigned int)recv_data[3] & 0xff) << std::setw(2) << std::setfill('0') << ((unsigned int)recv_data[4] & 0xff) << std::setw(2) << std::setfill('0') << ((unsigned int)recv_data[5] & 0xff);
            std::string gateway = ss.str();
            boost::to_upper(gateway);
            dev_status d;
            bool changed = false;
            if (wait_) {//was response to command we were waiting for
              std::cerr << "Insteon: Connected to gateway " << gateway << ",";
              unsigned char ack = (recv_data[6] & 0xf0) >> 1;
              unsigned char hops = (recv_data[6] & 0x0f);
              unsigned char delta = recv_data[7];
              std::cerr << " ACK code " << (int)ack << ", Hop Count " << (int)hops << ", Delta " << (int) delta << ",";
              d.level = (unsigned int)recv_data[8];
              std::cerr << " Level " << std::dec <<  d.level << "%" <<  std::endl;
              wait_ = false;
              changed = true;//if we updated from status we definitely want to use it
              need_delay = true;
            } else { //was command generated from user interaction with device
              if (gateway != dev_id_) {//message was not intended for us, ignore
			    std::cerr << "Insteon: Got message for " << gateway << ", discarding" << std::endl;
                delete[] recv_data;
                cur_message_[0] = 0x00;
                cur_message_[1] = 0x00;
                continue;
              }
              std::cerr << "Insteon: Processing message" << std::endl;
              unsigned char type_flag = (recv_data[6] & 0xf0);
			  bool all_link = (type_flag & 0b10000000) && (type_flag & 0b01000000);
              unsigned char hops = (recv_data[6] & 0x0f);
              unsigned char cmd = recv_data[7];
              unsigned char params = recv_data[8];

              switch (cmd) {
                case 0x12:
                  d.dbl = true;
                  std::cerr << "Insteon: Double Click" << std::endl;                  
                case 0x11:
                  std::cerr << "Insteon: Button On (" << (unsigned int)params << ")" << std::endl;
                  d.target = params;
                  d.level = 0xff;
                  changed = true;
                  break;
                case 0x14:
                  d.dbl = false;
                  std::cerr << "Insteon: Double Click" << std::endl;                  
                case 0x13:
                  std::cerr << "Insteon: Button Off (" << (unsigned int)params << ")" << std::endl;
                  d.target = params;
                  d.level = 0x00;
                  changed = true;
                  break;
                case 0x17: {
                  auto dt = dimming_status_.find(it->second);
                  if (dt != dimming_status_.end()) {
                    if (params == 0x00) {
                      std::cerr << "Insteon: Started dimming" << std::endl;
                      dt->second = DimmerDimming;
                    } else if (params == 0x01) {
                      std::cerr << "Insteon: Started brightening" << std::endl;
                      dt->second = DimmerBrightening;
                    }
                  }
                  break;
                }
                case 0x18: {
                  auto dt = dimming_status_.find(it->second);
                  if (dt != dimming_status_.end()) {
                    std::cerr << "Insteon: Dimming operation ended" << std::endl;
                    dt->second = DimmerNone;
                  }
                  break;
                }
              }
            }
            if (changed) {
              std::unique_lock<std::mutex> lock(status_lock_);
              auto jt = status_.find(it->second);
              if (jt == status_.end() || d.level != jt->second.level) {
                status_[it->second] = d;
              }
              status_q_.send(std::pair<std::string, dev_status>(it->second, d));
			  std::cerr << "Insteon: Status change queued" << std::endl;
              lock.unlock();
            }
          } else {
            std::cerr << "Insteon: Message from unknown device, discarding." << std::endl;
			delete[] recv_data;
			recv_data = 0;
			recv_size = 0;
			cur_message_[0] = 0x00;
			cur_message_[1] = 0x00;
          }
        } else if (cur_message_[1] == 0x51) {//Status extended command
          if (!buffer(&recv_data, recv_size, 23)) {
            continue;
          }
        } else if (cur_message_[1] == 0x52) {//X10 Command
          if (!buffer(&recv_data, recv_size, 2)) {
            //std::cerr << "Waiting for X10 Message" << std::endl;
            continue;
          }
        //std::cerr << "Got X10 Message" << std::endl;
        } else if (cur_message_[1] == 0x53) {//Link command
          if (!buffer(&recv_data, recv_size, 8)) {
            continue;
          }
        } else if (cur_message_[1] == 0x54) {//Seems to be sent on SET button press
          if (!buffer(&recv_data, recv_size, 1)) {
            continue;
          }
		  switch (recv_data[0]) {
			  case 0x02:
			  std::cerr << "Insteon: SET Button tapped" << std::endl;
			  break;
			  case 0x03:
			  std::cerr << "Insteon: SET Button held" << std::endl;
			  break;
			  case 0x04:
			  std::cerr << "Insteon: SET Button released after hold" << std::endl;
			  break;
			  case 0x12:
			  std::cerr << "Insteon: SET Button 2 tapped" << std::endl;
			  break;
			  case 0x13:
			  std::cerr << "Insteon: SET Button 2 held" << std::endl;
			  break;
			  case 0x14:
			  std::cerr << "Insteon: SET Button 2 released after hold" << std::endl;
			  break;
			  case 0x22:
			  std::cerr << "Insteon: SET Button 3 tapped" << std::endl;
			  break;
			  case 0x23:
			  std::cerr << "Insteon: SET Button 3 held" << std::endl;
			  break;
			  case 0x24:
			  std::cerr << "Insteon: SET Button 3 released after hold" << std::endl;
			  break;
		  }
        } else if (cur_message_[1] == 0x57) {//Link record
          if (!buffer(&recv_data, recv_size, 8)) {
            continue;
          }
        } else if (cur_message_[1] == 0x58) {//All-Link Cleanup Status Report
          if (!buffer(&recv_data, recv_size, 1)) {
            continue;
          }
        } else if (cur_message_[1] == 0x60) {//Get IM Info
          if (!buffer(&recv_data, recv_size, 7)) {
            continue;
          }
        } else if (cur_message_[1] == 0x61) {//ALL-Link Command
          if (!buffer(&recv_data, recv_size, 3)) {
            continue;
          }
        } else if (cur_message_[1] == 0x62) {//Direct Command
          if (!buffer(&recv_data, recv_size, 6)) {
            continue;
          }
        } else if (cur_message_[1] == 0x64) {
          if (!buffer(&recv_data, recv_size, 3)) {
            continue;
          }
        } else if (cur_message_[1] == 0x65) {
          if (!buffer(&recv_data, recv_size, 2)) {
            continue;
          }
        } else if (cur_message_[1] == 0x69 || cur_message_[1] == 0x6A) {
          if (!buffer(&recv_data, recv_size, 2)) {
            continue;
          }
        } else if (cur_message_[1] == 0x6F) {
          if (!buffer(&recv_data, recv_size, 10)) {
            continue;
          }
        }
        cur_message_[0] = 0x00;
        cur_message_[1] = 0x00;
      } else {
        buffer_.push_front(cur_message_[1]);
      }
      delete[] recv_data;
	  recv_data = 0;
	  recv_size = 0;
    }
    if (!last_command_data_ && !wait_) {
      if (need_delay && delay_ > 0) {
        usleep(delay_);//need to have a wait before sending next command
      }
      update_status();
    }
  }
}

void Insteon::run_dimmer() {
  while (!exit_) {
    usleep(500000);
    for (auto it = dimming_status_.begin(); it != dimming_status_.end(); it++) {
      if (it->second != DimmerNone) {
        std::unique_lock<std::mutex> lock(status_lock_);
        dev_status d;
        auto st = status_.find(it->first);
        if (st != status_.end()) {
          d = st->second;
        }
        if (it->second == DimmerDimming) {
          if (st == status_.end()) {
            d.level = 0xff;
          } else {
            d.level -= 30;
            if (d.level > 100) d.level = 0;
          }
        } else if (it->second == DimmerBrightening) {
          if (st == status_.end()) {
            d.level = 0;
          } else {
            d.level += 30;
            if (d.level > 0xff) d.level = 0xff;
          }
        }
        status_[it->first] = d;
        status_q_.send(std::pair<std::string, dev_status>(it->first, d));
      }
    }
  }
}
