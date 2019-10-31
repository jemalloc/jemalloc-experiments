cmake_minimum_required(VERSION 3.1)

find_package(gflags REQUIRED)
find_package(Threads)

add_executable(memsetVsMadvise memsetVsMadvise.cpp)
set_property(TARGET memsetVsMadvise PROPERTY CXX_STANDARD 11)
target_link_libraries(memsetVsMadvise PRIVATE ${CMAKE_THREAD_LIBS_INIT} gflags)

add_library(jemalloc STATIC IMPORTED)
set_target_properties(jemalloc PROPERTIES IMPORTED_LOCATION /usr/local/lib/libjemalloc.a)
target_link_libraries(jemalloc INTERFACE dl)

add_executable(stress stress_test/Main.cpp stress_test/Producers.cpp stress_test/Mixer.cpp stress_test/ThreadObject.cpp stress_test/Distribution.cpp stress_test/Allocation.cpp)
set_property(TARGET stress PROPERTY CXX_STANDARD 14)
target_link_libraries(stress PRIVATE gflags jemalloc m ${CMAKE_THREAD_LIBS_INIT})

add_executable(fillFlush fillFlush.cpp)
set_property(TARGET fillFlush PROPERTY CXX_STANDARD 11)
target_link_libraries(fillFlush PRIVATE gflags jemalloc ${CMAKE_THREAD_LIBS_INIT})
