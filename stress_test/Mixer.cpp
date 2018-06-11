#include "Mixer.h"

#include <gflags/gflags.h>
#include <jemalloc/jemalloc.h>

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

void Mixer::run() {
  while (this->producersRemaining_ > 0) {
    this->toFreeQueues_[this->me_]->free();
		// otherwise run a random producer
		Allocation a = this->pickProducer().run();
		if (!a.isEmpty()) {
			this->pickConsumer().addToFree(std::move(a));
		}
		producersRemaining_--;
  }
	je_mallctl("thread.tcache.flush", NULL, NULL, NULL, 0);
	// Main loop will eventually cleanup memory
}
