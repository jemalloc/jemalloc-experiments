#pragma once

#include <random>
#include <vector>

#include "Producers.h"
#include "ToFreeQueue.h"

using std::shared_ptr;
using std::vector;

class Mixer {
public:
  void run();
  Mixer(vector<shared_ptr<Producer>> producers, int numProducers, int me,
        vector<shared_ptr<ToFreeQueue>> toFreeQueues);

private:
  /* maintains reverse-sorted order by lifetime; i.e. [push_back] yields the
   * allocation that should be deallocated the soonest */
  vector<Allocation> allocated_;
  vector<shared_ptr<Producer>> producers_;
  int producersRemaining_;
  // the thread id that this mixer is running on
  int me_;
  // work queues for each thread indexed by thread number
  vector<shared_ptr<ToFreeQueue>> toFreeQueues_;
  // Picks next producer for the mixer to run. Currently uniform random choice.
  const Producer &pickProducer();
  // Picks a consumer to free memory allocated by a producer. Currently uniform
  // random choice.
  ToFreeQueue &pickConsumer();
  std::uniform_int_distribution<int> producerIdPicker_;
  std::uniform_int_distribution<int> consumerIdPicker_;
  std::default_random_engine generator_;
};
