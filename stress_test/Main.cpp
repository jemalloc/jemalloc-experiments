#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <gflags/gflags.h>
#include <jemalloc/jemalloc.h>

#include "Mixer.h"
#include "Distribution.h"

DEFINE_int32(num_producers, 1000, "number of producers to run on each thread");
DEFINE_int32(num_threads, 1, "number of threads to run");
DEFINE_bool(print_malloc_stats, false, "print out malloc stats after running");
DEFINE_string(distribution_file, "", "path to distribution file"); 
static bool validateDistributionFile(const char *flagName, const std::string &val) {
	return val.length() != 0;
}
DEFINE_validator(distribution_file, &validateDistributionFile);

using std::shared_ptr;
using std::make_shared;
using std::vector;

void createAndRunMixer(const Distribution &distr, int me,
                       vector<shared_ptr<ToFreeQueue>> toFreeQueues) {
  Mixer m(FLAGS_num_producers, distr, me, toFreeQueues);
  m.run();
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
	Distribution distr = parseDistribution(FLAGS_distribution_file.c_str());

	// Set up a work queue for each thread
  vector<std::thread> threads;
  vector<shared_ptr<ToFreeQueue>> toFreeQueues;
  for (int i = 0; i < FLAGS_num_threads; i++) {
    shared_ptr<ToFreeQueue> toFreeQ = make_shared<ToFreeQueue>();
    toFreeQueues.push_back(toFreeQ);
  }

  for (int i = 0; i < FLAGS_num_threads; i++) {
		// each thread gets an arbitrary id given by [i]
    threads.push_back(std::thread(createAndRunMixer, distr, i, toFreeQueues));
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

	if (FLAGS_print_malloc_stats) {
		je_malloc_stats_print(NULL, NULL, NULL);
	}

  std::cout << "Elapsed time: " << span.count() << std::endl;
}
