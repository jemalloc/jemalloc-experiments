cmake_minimum_required(VERSION 3.1)

find_package(gflags REQUIRED)
find_package(Threads)

add_executable(memsetVsMadvise memsetVsMadvise.cpp)
set_property(TARGET memsetVsMadvise PROPERTY CXX_STANDARD 11)
target_link_libraries(memsetVsMadvise PRIVATE ${CMAKE_THREAD_LIBS_INIT} gflags)

SET(JEMALLOC_COMPILE_FLAGS "-L/usr/local/lib")
SET(JEMALLOC_LINK_FLAGS "-ljemalloc -lm -lpthread")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${JEMALLOC_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${JEMALLOC_LINK_FLAGS}")
add_executable(stress stress_test/Main.cpp stress_test/Producers.cpp stress_test/Mixer.cpp stress_test/ToFreeQueue.cpp stress_test/Distribution.cpp)
set_property(TARGET stress PROPERTY CXX_STANDARD 14)
target_link_libraries(stress PRIVATE gflags)
