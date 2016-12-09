#pragma once

#include <atomic>
#include <mutex> 
#include "threadsafe_queue.h"

template <typename T>
class Task 
{
private:
	threadsafe_queue<T> &src;
	threadsafe_queue<T> &dst;

	enum { RUN = 0, WAIT, EXIT, };

	std::thread worker;
	std::mutex worker_mutex;
	std::atomic<int> worker_flag;
	std::condition_variable worker_conditional;
	std::once_flag worker_once_flag;

	void worker_run() {
		worker_flag.store(WAIT, std::memory_order_release);
		worker = std::thread(&Task<T>::job, this);
	}

public:
	explicit Task(threadsafe_queue<T> &src_, threadsafe_queue<T> &dst_) :
		src(src_), dst(dst_) {
		std::call_once(worker_once_flag, &Task::worker_run, this);
	}

	virtual ~Task() {
		worker_flag.store(EXIT, std::memory_order_release);
		if (worker.joinable())
			worker.join();
	}

	virtual void job() {

		printf("START\n");

		while(true) {
			std::unique_lock<std::mutex> lk(worker_mutex);
			worker_conditional.wait(lk, [this]()->bool { return (worker_flag.load() != WAIT); });
			lk.unlock();

			int src_value;
			int dst_value;

			src.wait_and_pop(src_value);
			process(src_value, dst_value);			
			dst.push(dst_value);

			lk.lock();
			if (worker_flag.load() == EXIT)
			{
				printf("QUIT\n");
				return;
			}				
		}
	}

	void start() {
		if (RUN == worker_flag.load())
			return;

		std::lock_guard<std::mutex> lk(worker_mutex);
		worker_flag.store(RUN, std::memory_order_release);
		worker_conditional.notify_one();
	}

	void stop() {
		if (EXIT == worker_flag.load())
			return;

		std::lock_guard<std::mutex> lk(worker_mutex);
		worker_flag.store(EXIT, std::memory_order_release);
	}

	void pause() {
		if (WAIT == worker_flag.load())
			return;

		std::lock_guard<std::mutex> lk(worker_mutex);
		worker_flag.store(WAIT, std::memory_order_release);
	}

	/* It is not abstarct functions since it might be called in 
	 * worker thread during destroying */
	virtual void process(int &src_value, int &dst_value) {}
};

template <typename T>
class TaskMulTwo : public Task<T>
{
public:
	TaskMulTwo(threadsafe_queue<T> &src_, threadsafe_queue<T> &dst_) :
		Task<T>(src_, dst_){}

	~TaskMulTwo() {}
	
	virtual void process(int &src_value, int &dst_value) {
		//printf("TaskMulTwo: %d\n", src_value);
		dst_value = src_value*2;
	}
};

template <typename T>
class TaskMulFore : public Task<T>
{
public:
	TaskMulFore(threadsafe_queue<T> &src_, threadsafe_queue<T> &dst_) :
		Task<T>(src_, dst_){}
	
	~TaskMulFore() {}
	
	virtual void process(int &src_value, int &dst_value) {
		//printf("TaskMulFore: %d\n", src_value);
		dst_value = src_value*4;
	}
};
