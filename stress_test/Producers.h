#pragma once

#include <chrono>
#include <vector>

class Allocation {
public:
  // sorts based on [freeAfter] field
  bool operator<(const Allocation &that) const;
  bool operator>(const Allocation &that) const;
  // true iff [this->toFree_] is empty
  bool isEmpty() const;
  std::chrono::high_resolution_clock::time_point freeAfter() const;
  Allocation(std::vector<void *> toFree,
             std::chrono::high_resolution_clock::time_point freeAfter);
  // makes an allocation such that [isEmpty()] is true
  Allocation() = default;

  // Disable copy constructor: whoever owns the Allocation should deallocate it
  Allocation(Allocation const &) = delete;
  Allocation &operator=(Allocation const &) = delete;

	// must define a move constructor since we deleted the copy constructor
	Allocation(Allocation&&) = default;
	Allocation& operator=(Allocation&&) = default;

	// The destructor deallocates the memory in [toFree_]
  ~Allocation();

	// needed to sort
	friend void swap(Allocation &a1, Allocation &a2);

private:
  std::vector<void *> toFree_;
  // absolute time after which this should be freed
  std::chrono::high_resolution_clock::time_point freeAfter_;
};

class Producer {
public:
  virtual Allocation run() const = 0;
};

// allocates a vector of size [sz] and then frees it
class VectorProducer : public Producer {
public:
  Allocation run() const;
  // allocate, and then free after [lifetime] has elapsed
  VectorProducer(int vectorSize, std::chrono::duration<double> lifetime);
  // allocate and then free immediately
  VectorProducer(int vectorSize);

private:
  int vectorSize_;
  std::chrono::duration<double> lifetime_;
  bool shouldFree_;
};

/* allocates a block of size [alloc_sz], and then immediately frees it. Repeats
 * this [n_allocs] times. */
class SimpleProducer : public Producer {
public:
  Allocation run() const;
  SimpleProducer(int allocSize, int numAllocs);

private:
  int allocSize_;
  int numAllocs_;
};
