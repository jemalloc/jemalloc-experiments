#include <sys/mman.h>

#include <stdlib.h>

#include <cstdint>
#include <cstring>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include <gflags/gflags.h>

#include "util.h"

DEFINE_int32(num_runs, 100,
    "Number of times to zero the pages (per page count)");
DEFINE_int32(max_pages, 50, "Maximum number of pages to zero");
DEFINE_int32(num_threads, 1,
    "Number of threads on which to try the experiment at once.");
DEFINE_bool(touch_after_zero, false,
    "Whether to actually try touching the pages we zero.");

struct Result {
  std::uint64_t memsetCycles;
  std::uint64_t madviseCycles;

  void accum(const Result& other) {
    memsetCycles += other.memsetCycles;
    madviseCycles += other.madviseCycles;
  }
};

void maybeTouchPages(char* begin, std::size_t length) {
  if (FLAGS_touch_after_zero) {
    for (char* ptr = begin; ptr != begin + length; ptr += 4096) {
      *ptr = 0;
    }
  }
}

Result runTest(std::size_t size) {
  Result result;
  void *ptrv;
  int err = posix_memalign(&ptrv, 4096, size);
  if (err != 0) {
    std::cerr << "Couldn't allocate; error was " << err << std::endl;
    exit(1);
  }
  char* ptr = static_cast<char*>(ptrv);
  // Touch all the pages from this thread.
  std::memset(ptrv, 0, size);
  // Touch all the pages from another thread.
  std::thread child([=]() {
    std::memset(ptrv, 0, size);
  });
  child.join();
  // We'll probably be dealing with uncached memory here; we care about this
  // difference when pulling memory out of an inactive state.
  util::flushCache(ptr, ptr + size);
  result.memsetCycles = util::runTimed([&]() {
    std::memset(ptrv, 0, size);
    maybeTouchPages(ptr, size);
  });
  util::flushCache(ptr, ptr + size);
  result.madviseCycles = util::runTimed([&]() {
    err = madvise(ptr, size, MADV_DONTNEED);
    if (err != 0) {
      std::cerr << "Couldn't madvise; error was " << err << std::endl;
      exit(1);
    }
    maybeTouchPages(ptr, size);
  });

  return result;
}

int main(int argc, char** argv) {
  std::string usage =
    "This program benchmarks memset vs madvise for zeroing memory.\n"
    "Sample usage:\n";
  usage += argv[0];
  usage += " --max_pages=20 --num_runs=30 --num_threads=4 ";
  usage += "--touch_after_zero=true";

  gflags::SetUsageMessage(usage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  for (int i = 1; i <= FLAGS_max_pages; ++i) {
    Result sum = {0, 0};
    for (int j = 0; j < FLAGS_num_runs; ++j) {
      std::vector<std::future<Result>> results;
      for (int k = 0; k < FLAGS_num_threads; ++k) {
        results.push_back(std::async(std::launch::async, runTest, 4096 * i));
      }
      for (int k = 0; k < FLAGS_num_threads; ++k) {
        sum.accum(results[k].get());
      }
    }
    std::cout << "When zeroing " << i << " pages (averaging across "
      << FLAGS_num_runs << " runs of " << FLAGS_num_threads << " threads:\n"
      << "    memset:  " << sum.memsetCycles / FLAGS_num_runs << " cycles\n"
      << "    madvise: " << sum.madviseCycles / FLAGS_num_runs << " cycles\n";
  }

  return 0;
}
