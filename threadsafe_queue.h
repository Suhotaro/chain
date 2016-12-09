#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

template<typename T>
class threadsafe_queue {
private:
	std::mutex m;
	std::queue<T> data_queue;
	std::condition_variable data_cond;

public:
	void push(T new_value) {
		std::lock_guard<std::mutex> lk(m);
		data_queue.push(new_value);
		data_cond.notify_one();
	}

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(m);
		data_cond.wait(lk, [this]{ return !data_queue.empty(); });
		value = data_queue.front();
		data_queue.pop();
	}
};