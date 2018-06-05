#pragma once

#include <queue>
#include <thread>

#include "Producers.h"

class ToFreeQueue {
public:
	// frees all allocations whose lifetime has elapsed
	void free();
	// free all allocations, even if the lifetime hasn't expired
	void freeIgnoreLifetime();
	// Add an allocation to be freed after a particular time
  void addToFree(Allocation a);

private:
	std::mutex lock_;
	std::priority_queue<Allocation, std::vector<Allocation>, std::greater<Allocation>> q_;
};
