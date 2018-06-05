#include "Mixer.h"

Mixer::Mixer(vector<shared_ptr<Producer>> producers, int numProducers, int me,
             vector<shared_ptr<ToFreeQueue>> toFreeQueues)
    : producers_(producers), producersRemaining_(numProducers),
			toFreeQueues_(toFreeQueues), me_(me),
      producerIdPicker_(0, producers.size() - 1),
			consumerIdPicker_(0, toFreeQueues.size() - 1)  {}

// Picks next producer for the mixer to run. Currently uniform random choice
const Producer &Mixer::pickProducer() {
  int producerIndex = this->producerIdPicker_(this->generator_);
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
	// Main loop will eventually cleanup memory
}
