#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <gflags/gflags.h>

#include "Mixer.h"

DEFINE_int32(num_producers, 100, "number of producers to run");
DEFINE_int32(num_threads, 1, "number of threads to run");

using std::shared_ptr;
using std::make_shared;
using std::vector;

void createAndRunMixer(vector<shared_ptr<Producer>> producers, int me,
                       vector<shared_ptr<ToFreeQueue>> toFreeQueues) {
  Mixer m(producers, FLAGS_num_producers, me, toFreeQueues);
  m.run();
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

	// Initialize producers
  vector<shared_ptr<Producer>> producers;
  producers.push_back(make_shared<SimpleProducer>(8, 100000));
  producers.push_back(make_shared<VectorProducer>(100000, std::chrono::duration<double>(1.0)));
  producers.push_back(make_shared<LinkedListProducer>(8, 100000, std::chrono::duration<double>(1.0)));

	// Set up a work queue for each thread
  vector<std::thread> threads;
  vector<shared_ptr<ToFreeQueue>> toFreeQueues;
  for (int i = 0; i < FLAGS_num_threads; i++) {
    shared_ptr<ToFreeQueue> toFreeQ = make_shared<ToFreeQueue>();
    toFreeQueues.push_back(toFreeQ);
  }

  for (int i = 0; i < FLAGS_num_threads; i++) {
		// each thread gets an arbitrary id given by [i]
    threads.push_back(std::thread(createAndRunMixer, producers, i, toFreeQueues));
  }

  using namespace std::chrono;

  high_resolution_clock::time_point beginTime = high_resolution_clock::now();
  for (auto it = begin(threads); it != end(threads); ++it) {
    it->join();
  }
	// Cleanup any remaining memory
	for (int i = 0; i < FLAGS_num_threads; i++) {
		toFreeQueues[i]->freeIgnoreLifetime();	
	}
  high_resolution_clock::time_point endTime = high_resolution_clock::now();

  duration<double> span = duration_cast<duration<double>>(endTime - beginTime);
  std::cout << "Elapsed time: " << span.count() << std::endl;
}
