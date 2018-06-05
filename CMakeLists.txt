cmake_minimum_required(VERSION 3.1)

find_package(gflags REQUIRED)
find_package(Threads)

add_executable(memsetVsMadvise memsetVsMadvise.cpp)
set_property(TARGET memsetVsMadvise PROPERTY CXX_STANDARD 11)
target_link_libraries(memsetVsMadvise PRIVATE ${CMAKE_THREAD_LIBS_INIT} gflags)

add_executable(stress stress_test/Main.cpp stress_test/Producers.cpp stress_test/Mixer.cpp stress_test/ToFreeQueue.cpp)
set_property(TARGET stress PROPERTY CXX_STANDARD 14)
target_link_libraries(stress PRIVATE gflags)
