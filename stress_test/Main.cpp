#include <chrono>
#include <iostream>
#include <vector>

#include <gflags/gflags.h>

#include "Mixer.h"

DEFINE_int32(num_producers, 100, "number of producers to run");

int main(int argc, char **argv) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  vector<std::unique_ptr<Producer>> producers;
  producers.push_back(std::move(std::make_unique<SimpleProducer>(8, 100000)));
  producers.push_back(std::move(std::make_unique<VectorProducer>(
      100000, std::chrono::duration<double>(1.0))));

  using namespace std::chrono;

  Mixer m(std::move(producers), FLAGS_num_producers);

  high_resolution_clock::time_point beginTime = high_resolution_clock::now();
  m.run();
  high_resolution_clock::time_point endTime = high_resolution_clock::now();

  duration<double> span = duration_cast<duration<double>>(endTime - beginTime);
  std::cout << "Elapsed time: " << span.count() << std::endl;
}
