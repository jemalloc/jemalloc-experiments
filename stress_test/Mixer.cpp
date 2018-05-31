#include "Mixer.h"

Mixer::Mixer(vector<unique_ptr<Producer>> producers, int numProducers)
    : producers_(move(producers)), producersRemaining_(numProducers),
      producerIndexPicker_(0, this->producers_.size() - 1) {}

// Picks next producer for the mixer to run. Currently uniform random choice
const Producer &Mixer::pick() {
  int producerIndex = this->producerIndexPicker_(this->generator_);
  return *(this->producers_[producerIndex]);
}

void Mixer::run() {
  while (this->producersRemaining_ > 0) {
    if (this->allocated_.size() > 0 &&
        this->allocated_.back().freeAfter() <
            std::chrono::high_resolution_clock::now()) {
			// deallocate something if it's lifetime has expired
      this->allocated_.pop_back();
    } else {
			// otherwise run a random producer
      Allocation a = this->pick().run();
      if (!a.isEmpty()) {
        this->allocated_.push_back(std::move(a));
        sort(rbegin(this->allocated_), rend(this->allocated_));
      }
      producersRemaining_--;
    }
  }
  // cleanup remaining allocated things immediately, regardless of lifetime
  while (!this->allocated_.empty()) {
    this->allocated_.pop_back();
  }
}
