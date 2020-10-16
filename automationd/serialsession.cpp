/*
 * serialsession.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: Marc
 */

#include "serialsession.h"
#ifdef __linux__
#include <termios.h>
#include <string.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <iostream>
#include <atomic>
#include "msgq.h"
#include <iomanip>
#include <list>

#define DEFAULT_BUFFER      2

class SerialSession_impl {
  enum DATA_DIR {
    DATA_UNKNOWN = -1,
    DATA_IN = 0,
    DATA_OUT = 1
  };
  struct data_q_t {
      data_q_t() {
        direction = DATA_UNKNOWN;
        data = 0;
        size = 0;
      }
      DATA_DIR direction;
      unsigned char* data;
      size_t size;
  };
  public:
    SerialSession_impl(const std::string& dev) {
      dev_ = dev;
      open();
      exit_ = false;
      recv_ = new std::thread(&SerialSession_impl::recv_loop, std::ref(*this));
      send_ = new std::thread(&SerialSession_impl::send_loop, std::ref(*this));
    }
    ~SerialSession_impl() {
      exit_ = true;
      recv_->join();
      delete recv_;
      send_->join();
      delete send_;
#ifdef __linux__
      close(fd_);
#endif
    }
    void send_packet(unsigned char* data, size_t size) {
      data_q_t d;
      d.direction = DATA_OUT;
      d.data = new unsigned char[size];
      memcpy(d.data, data, size);
      d.size = size;
      send_queue_.send(d);
    }
    unsigned char* recv_packet(size_t& size) {
      data_q_t item;
      if (recv_queue_.get(item, std::chrono::microseconds(10000))) {
        size = item.size;
        return item.data;
      } else {
        return 0;
      }
    }
    bool connected() const {
      return true;
    }
  private:
    msgq_t<data_q_t> send_queue_;
    msgq_t<data_q_t> recv_queue_;
    std::list<char> buffer_;
    int fd_;
    struct termios tty_;
    std::thread* recv_;
    std::thread* send_;
    std::atomic<bool> exit_;
    std::string dev_;

    void open() {
#ifdef __linux__
      fd_ = ::open(dev_.c_str(), O_RDWR | O_NOCTTY);
      memset(&tty_, 0, sizeof(tty_));
      if (tcgetattr(fd_, &tty_) != 0) {
        std::cerr << "Error on tcgetattr" << std::endl;
      }

      cfsetospeed(&tty_, (speed_t)B19200);
      cfsetispeed(&tty_, (speed_t)B19200);

      tty_.c_cflag &= ~PARENB;
      tty_.c_cflag &= ~CSTOPB;
      tty_.c_cflag &= ~CSIZE;
      tty_.c_cflag |= CS8;

      tty_.c_cflag &= ~CRTSCTS;
      tty_.c_cc[VMIN] = 0;
      tty_.c_cc[VTIME] = 0;

      tty_.c_cflag |=  CREAD | CLOCAL;

      cfmakeraw(&tty_);
      tcflush(fd_, TCIFLUSH);
      if (tcsetattr(fd_, TCSANOW, &tty_) != 0) {
        std::cerr << "Error from tcsetattr" << std::endl;
      }
#endif
    }

    void recv_loop() {
      unsigned char buf[DEFAULT_BUFFER];
      int ret = -1;
      while(1)
      {
          ret = read(fd_, buf, DEFAULT_BUFFER);
          //std::cerr << "Recv: ";
          //for (int i = 0; i < ret; i++) {
          //  std::cerr << std::setw(2) << std::setfill('0') << std::hex << (((unsigned short)buf[i]) & 0xff);
          //}
          //std::cerr << std::endl;
          if (ret < 0) {
            std::cerr << "Error reading from tty." << std::endl;
          } else {
            data_q_t d;
            d.data = new unsigned char[ret];
            memcpy(d.data, buf, ret);
            d.size = ret;
            d.direction = DATA_IN;
            recv_queue_.send(d);
          }
          if (exit_) {
            return;
          }
      }
    }
    void send_loop() {
      while (1) {
        data_q_t item;
        if (send_queue_.get(item, std::chrono::microseconds(1000000))) {
          int ret = write(fd_, (char*)item.data, item.size);
          //std::cerr << "Send: ";
          //for (size_t i = 0; i < item.size; i++) {
          //  std::cerr << std::setw(2) << std::setfill('0') << std::hex << (((unsigned short)item.data[i]) & 0xff);
          //}
          //std::cerr << std::endl;
          delete[] item.data;
          if (ret < 0) {
            std::cerr << "Error writing to tty" << std::endl;
          }
        }
        if (exit_) {
          return;
        }
      }
    }
};

SerialSession::SerialSession(const std::string& dev) {
  pimpl_ = new SerialSession_impl(dev);
}

SerialSession::~SerialSession() {
  delete pimpl_;
}

bool SerialSession::connected() const {
  return pimpl_->connected();
}

void SerialSession::send_packet(unsigned char* data, size_t size) {
  pimpl_->send_packet(data, size);
}
unsigned char* SerialSession::recv_packet(size_t& size) {
  return pimpl_->recv_packet(size);
}
