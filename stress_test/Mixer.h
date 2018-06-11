#pragma once

#include <random>
#include <vector>

#include "Producers.h"
#include "ToFreeQueue.h"
#include "Distribution.h"

class Mixer {
public:
  void run();
  Mixer(int numProducers, const Distribution &distr, int me,
        std::vector<std::shared_ptr<ToFreeQueue>> toFreeQueues);

private:
  int producersRemaining_;
  // the thread id that this mixer is running on
  int me_;
  // work queues for each thread indexed by thread number
  std::vector<std::shared_ptr<ToFreeQueue>> toFreeQueues_;
  // Picks next producer for the mixer to run
  const Producer &pickProducer();
  /* Picks a consumer to free memory allocated by a producer. Currently uniform
   * random choice */
  ToFreeQueue &pickConsumer();

  std::uniform_int_distribution<int> consumerIdPicker_;
  std::default_random_engine generator_;

	// for picking producer with weighted random choice
	std::vector<double> weightArray_;
	double totalWeight_;
  std::vector<std::unique_ptr<Producer>> producers_;
	std::uniform_real_distribution<double> producerWeightPicker_;
	// initializes [producers_], [totalWeight_], [weightArray_], and [producerWeightPicker_]
	void initProducers(const Distribution &distr);
	void addProducer(double weight, std::unique_ptr<Producer> p);
};
