/*
 * session.cpp
 *
 *  Created on: Sep 17, 2014
 *      Author: marc.micalizzi
 */

#include "tcpsession.h"
#ifdef _WIN32
#include <winsock2.h>
#define lasterror WSAGetLastError()
#elif defined(__linux__)
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#endif
#include <thread>
#include <iostream>
#include <atomic>
#include "msgq.h"
#include <iomanip>

#define DEFAULT_BUFFER      4096

class TcpSession_impl {
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
    TcpSession_impl(const struct sockaddr_in& sock, int fd) {
      sock_ = sock;
      client_ = fd;
      connected_ = true;
      exit_ = false;
      recv_ = new std::thread(&TcpSession_impl::recv_loop, std::ref(*this));
      send_ = new std::thread(&TcpSession_impl::send_loop, std::ref(*this));
    }
    ~TcpSession_impl() {
      exit_ = true;
      recv_->join();
      delete recv_;
      send_->join();
      delete send_;
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
      }
      return 0;
    }
    bool connected() const {
      return connected_;
    }
  private:
    msgq_t<data_q_t> send_queue_;
    msgq_t<data_q_t> recv_queue_;
    std::list<char> buffer_;
    struct sockaddr_in sock_;
    int client_;
    std::thread* recv_;
    std::thread* send_;
    std::atomic<bool> exit_;
    std::atomic<bool> connected_;
    void recv_loop() {
      unsigned char buf[DEFAULT_BUFFER];
      int ret = -1;
      while(1)
      {
          ret = recv(client_, (char*)buf, DEFAULT_BUFFER, 0);
          //std::cerr << "Recv: ";
          //for (int i = 0; i < ret; i++) {
          //  std::cerr << std::setw(2) << std::setfill('0') << std::hex << (((unsigned short)buf[i]) & 0xff);
          //  buffer_.push_back(buf[i]);
          //}
          //std::cerr << std::endl;
          if (ret < 0) {
            std::cerr << "Connection reset." << std::endl;
            connected_ = false;
            return;
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
        if (connected_) {
          if (send_queue_.get(item, std::chrono::microseconds(1000000))) {
             int ret = send(client_, (char*)item.data, item.size, 0);
             //std::cerr << "Send: ";
             //for (size_t i = 0; i < item.size; i++) {
             //  std::cerr << std::setw(2) << std::setfill('0') << std::hex << (((unsigned short)item.data[i]) & 0xff);
             //}
             //std::cerr << std::endl;
             delete[] item.data;
             if (ret < 0) {
               std::cerr << "Error sending data" << std::endl;
               connected_ = false;
               return;
             }
          }
        } else {
          usleep(100000);
        }
        if (exit_) {
          return;
        }
      }
    }
};

TcpSession::TcpSession(const struct sockaddr_in& sock, int fd) {
  pimpl_ = new TcpSession_impl(sock, fd);
}

TcpSession::~TcpSession() {
  delete pimpl_;
}

bool TcpSession::connected() const {
  return pimpl_->connected();
}

void TcpSession::send_packet(unsigned char* data, size_t size) {
  pimpl_->send_packet(data, size);
}
unsigned char* TcpSession::recv_packet(size_t& size) {
  return pimpl_->recv_packet(size);
}

