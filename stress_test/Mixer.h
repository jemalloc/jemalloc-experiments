#pragma once

#include <random>
#include <vector>

#include "Producers.h"

using std::unique_ptr;
using std::vector;

class Mixer {
public:
  void run();
  Mixer(vector<unique_ptr<Producer>> producers, int numProducers);

private:
  /* maintains reverse-sorted order by lifetime; i.e. [push_back] yields the
   * allocation that should be deallocated the soonest */
  vector<Allocation> allocated_;
  vector<unique_ptr<Producer>> producers_;
  int producersRemaining_;
  const Producer &pick();
  std::uniform_int_distribution<int> producerIndexPicker_;
  std::default_random_engine generator_;
};
