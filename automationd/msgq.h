#ifndef MSGQ_H_
#define MSGQ_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>

template <class T>
class msgq_t {
public:
	msgq_t() {
	}
	~msgq_t() {
		while(!msg_q_.empty()) {
			if (msg_q_.front()) delete msg_q_.front();
			msg_q_.pop_front();
		}
	}
	void send(const T &msg) {
		lock_.lock();
		msg_q_.push_back(new T(msg));
		cond_.notify_one();
		lock_.unlock();
	}
	bool empty() {
		lock_.lock();
		bool rv = msg_q_.empty();
		lock_.unlock();
		return rv;
	}
	bool get(T &m, std::chrono::microseconds t = std::chrono::microseconds::zero()) {
		bool rv = false;
		std::unique_lock<std::mutex> lock(lock_);
		for(;;) {
			if(msg_q_.empty() && t != std::chrono::microseconds::zero()) {
			  if(t > std::chrono::microseconds::zero())
					cond_.wait_for(lock, t);
			  else
					cond_.wait(lock);
			}
			if(msg_q_.empty()) break;
			T* msg = msg_q_.front();
			msg_q_.pop_front();
			if(msg) {
				m = *msg;
				delete msg;
				rv = true;
				break;
			}
		}
		lock_.unlock();
		return rv;
	}
private:
	msgq_t(const msgq_t &);

	std::mutex lock_;
	std::condition_variable cond_;
	std::list<T*> msg_q_;
};


#endif

