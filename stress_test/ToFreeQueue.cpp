#include "ToFreeQueue.h"

#include <chrono>
#include <mutex>
#include <thread>

void ToFreeQueue::free() {
	std::lock_guard<std::mutex> guard(this->lock_);

  while (!this->q_.empty() &&
         this->q_.top().freeAfter() < std::chrono::high_resolution_clock::now()) {
    this->q_.pop();
  }
}

void ToFreeQueue::freeIgnoreLifetime() {
	std::lock_guard<std::mutex> guard(this->lock_);

	while (!this->q_.empty()) {
		this->q_.pop();
	}
}

void ToFreeQueue::addToFree(Allocation a) {
	std::lock_guard<std::mutex> guard(this->lock_);
  this->q_.push(std::move(a));
}
