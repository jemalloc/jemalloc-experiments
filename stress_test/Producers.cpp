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

VectorProducer::VectorProducer(size_t vectorSize,
                               std::chrono::duration<double> lifetime)
    : vectorSize_(vectorSize), lifetime_(lifetime) {}

std::chrono::high_resolution_clock::time_point addToNow(std::chrono::duration<double> d) {
	using namespace std::chrono;
	high_resolution_clock::time_point t = high_resolution_clock::now();
	high_resolution_clock::duration dHighResolution =
			duration_cast<high_resolution_clock::duration>(d);
	t += dHighResolution;;
	return t;
}

Allocation VectorProducer::run() const {
  void *ptr = malloc(1);
  size_t currSize = 1;
  while (currSize < this->vectorSize_) {
    free(ptr);
    currSize *= 2;
    ptr = malloc(currSize);
  }

	return std::move(Allocation(std::vector<void *>({ptr}), addToNow(this->lifetime_)));
}

// LinkedList Producer

Allocation LinkedListProducer::run() const {

	std::vector<void *> toFree;
	toFree.reserve(this->numNodes_);

	for (int i = 0; i < this->numNodes_; i++) {
		toFree.push_back(malloc(this->nodeSize_));
	}
	
	return std::move(Allocation(toFree, addToNow(this->lifetime_)));
}
// allocate [numNodes] blocks of size [nodeSize] with lifetime [lifetime]
LinkedListProducer::LinkedListProducer(size_t nodeSize, int numNodes, std::chrono::duration<double> lifetime) :
	nodeSize_(nodeSize), numNodes_(numNodes), lifetime_(lifetime) {}
