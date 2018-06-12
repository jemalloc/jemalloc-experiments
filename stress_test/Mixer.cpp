#include "Mixer.h"

#include <cstdint>
#include <iostream>

#include <gflags/gflags.h>
#include <jemalloc/jemalloc.h>
#include <stdlib.h>
#include <sys/mman.h>

DEFINE_int32(producer_duration, 10000, "scales the length of producers. Making"
		"this number higher means each producer runs for a long time.");

using std::make_unique;
using std::unique_ptr;
using std::shared_ptr;
using std::vector;

Mixer::Mixer(int numProducers, const Distribution &distr, int me,
						 vector<shared_ptr<ToFreeQueue>> toFreeQueues)
    : producersRemaining_(numProducers),
			toFreeQueues_(toFreeQueues), me_(me),
			consumerIdPicker_(0, toFreeQueues.size() - 1) {
	this->totalWeight_ = 0.0;
	this->initProducers(distr);	
	this->producerWeightPicker_ = std::uniform_real_distribution<double>(0.0, this->totalWeight_);
}

void Mixer::addProducer(double weight, unique_ptr<Producer> p) {
	this->totalWeight_ += weight;
	this->producers_.push_back(std::move(p));
	this->weightArray_.push_back(this->totalWeight_);
}

void Mixer::initProducers(const Distribution &distr) {
	auto oneSecond = std::chrono::duration<double>(1.0);

	std::uniform_int_distribution<int> vectorInitFuzzer(1, 100);

	for (auto it = begin(distr); it != end(distr); ++it) {
		addProducer(it->freq / 3.0,
			std::move(make_unique<SimpleProducer>(it->size, FLAGS_producer_duration)));
		// provide a bit of fuzziness to vector initial value
		int vectorInit = vectorInitFuzzer(this->generator_);
		addProducer(it->freq / 3.0,
			std::move(make_unique<VectorProducer>(FLAGS_producer_duration, oneSecond, vectorInit)));
		addProducer(it->freq / 3.0,
			std::move(make_unique<LinkedListProducer>(it->size, FLAGS_producer_duration, oneSecond)));
	}
}

const Producer &Mixer::pickProducer() {
	double r = this->producerWeightPicker_(this->generator_);
	int producerIndex;
	for (producerIndex = 0; producerIndex < this->weightArray_.size(); ++producerIndex) {
		if (r <= weightArray_[producerIndex]) {
			break;
		}
	}
	assert(producerIndex != this->weightArray_.size());
  return *(this->producers_[producerIndex]);
}

// Picks next producer for the mixer to run. Currently uniform random choice
ToFreeQueue& Mixer::pickConsumer() {
  int consumerIndex = this->consumerIdPicker_(this->generator_);
  return *(this->toFreeQueues_[consumerIndex]);
}

constexpr size_t kMaxDataCacheSize = 8000000;
constexpr size_t kMaxInstCacheSize = 32000;

static char dataBurner[kMaxDataCacheSize] = {0};
static char *instBurner = nullptr;

constexpr unsigned char instRet = { 0xC3 };
constexpr unsigned char instNop = { 0x90 };

void burnDataCache(size_t n) {
	// Do something slightly non-trivial so this doesn't get optimized away
	size_t nClipped = (n > kMaxDataCacheSize)? kMaxDataCacheSize : n;
	char c = dataBurner[0];
	for (int i = 0; i < nClipped; i++) {
		dataBurner[i] = c + 1;
	}
}

void burnInstCache(size_t n) {
	// initialize if null
	size_t sz = kMaxInstCacheSize + 1;
	if (instBurner == nullptr) {
		instBurner = (char*) mmap(NULL, sz, PROT_READ | PROT_WRITE,
															MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		char *p = instBurner;
		for (int i = 0; i < sz - 1; ++i) {
			*(p++) = instNop;
		}
		*(p++) = instRet;
		if (mprotect(instBurner, sz, PROT_NONE) == -1) {
			std::cout << "mprotect failed" << std::endl;
			exit(1);
		}
		if (mprotect(instBurner, sz, PROT_EXEC | PROT_READ) == -1) {
			std::cout << "mprotect failed" << std::endl;
			exit(1);
		}
	}

	int nClipped = (n > kMaxInstCacheSize)? kMaxInstCacheSize : n;
	int offset = kMaxInstCacheSize - nClipped;

	void (*f)() = (void (*)())(instBurner + offset);
	(*f)();
}

void Mixer::run() {
  while (this->producersRemaining_ > 0) {
    this->toFreeQueues_[this->me_]->free();
		// otherwise run a random producer
		Allocation a = this->pickProducer().run();
		burnInstCache(kMaxInstCacheSize);
		burnDataCache(kMaxDataCacheSize / 8);
		if (!a.isEmpty()) {
			this->pickConsumer().addToFree(std::move(a));
		}
		producersRemaining_--;
  }
	je_mallctl("thread.tcache.flush", NULL, NULL, NULL, 0);
	// Main loop will eventually cleanup memory
}
