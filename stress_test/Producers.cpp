#include "Producers.h"

#include <iostream>

// Allocation

bool Allocation::operator<(const Allocation &that) const {
  return this->toFree_ < that.toFree_;
}

bool Allocation::operator>(const Allocation &that) const {
	return !(*this < that);
}

bool Allocation::isEmpty() const { return this->toFree_.size() == 0; }

std::chrono::high_resolution_clock::time_point Allocation::freeAfter() const {
  return this->freeAfter_;
}

Allocation::Allocation(std::vector<void *> toFree,
                       std::chrono::high_resolution_clock::time_point freeAfter)
    : toFree_(toFree), freeAfter_(freeAfter) {}

Allocation::~Allocation() {
  for (auto it = begin(this->toFree_); it != end(this->toFree_); ++it) {
    free(*it);
  }
}

// Simple Producer

SimpleProducer::SimpleProducer(int allocSize, int numAllocs)
    : allocSize_(allocSize), numAllocs_(numAllocs) {}

Allocation SimpleProducer::run() const {
  for (int i = 0; i < this->numAllocs_; i++) {
    char *ptr = (char *)calloc(this->allocSize_, sizeof(char));
    if (ptr == NULL) {
      std::cout << "allocation failed" << std::endl;
    }
    free(ptr);
  }
  return std::move(Allocation());
}

void swap(Allocation &a1, Allocation &a2) {
	a1.toFree_.swap(a2.toFree_);
	std::swap(a1.freeAfter_, a2.freeAfter_);
}

// Vector Producer

VectorProducer::VectorProducer(int vectorSize,
                               std::chrono::duration<double> lifetime)
    : vectorSize_(vectorSize), lifetime_(lifetime), shouldFree_(true) {}

VectorProducer::VectorProducer(int vectorSize)
    : vectorSize_(vectorSize), lifetime_(0.0), shouldFree_(true) {}

Allocation VectorProducer::run() const {
  void *ptr = malloc(1);
  size_t currSize = 1;
  while (currSize < this->vectorSize_) {
    free(ptr);
    currSize *= 2;
    ptr = malloc(currSize);
  }
  if (this->shouldFree_) {
    free(ptr);
    return Allocation();
  } else {

    using namespace std::chrono;
    high_resolution_clock::time_point t = high_resolution_clock::now();
    high_resolution_clock::duration d =
        duration_cast<high_resolution_clock::duration>(this->lifetime_);
    t += d;
    return std::move(Allocation(std::vector<void *>({ptr}), t));
  }
}
