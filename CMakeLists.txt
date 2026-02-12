cmake_minimum_required(VERSION 3.16)
project(distributed_systems_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(dss
  core/event_engine.cpp
  network/network.cpp
  failures/failure_injector.cpp
  algorithms/raft/raft.cpp
  metrics/metrics.cpp
  experiments/config.cpp
  experiments/simulator.cpp
)
target_include_directories(dss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(run_experiment experiments/run_experiment.cpp)
target_link_libraries(run_experiment PRIVATE dss)

add_executable(test_event_engine tests/test_event_engine.cpp)
target_link_libraries(test_event_engine PRIVATE dss)
add_test(NAME test_event_engine COMMAND test_event_engine)

add_executable(test_clocks_and_network tests/test_clocks_and_network.cpp)
target_link_libraries(test_clocks_and_network PRIVATE dss)
add_test(NAME test_clocks_and_network COMMAND test_clocks_and_network)

add_executable(test_raft_and_snapshot tests/test_raft_and_snapshot.cpp)
target_link_libraries(test_raft_and_snapshot PRIVATE dss)
add_test(NAME test_raft_and_snapshot COMMAND test_raft_and_snapshot)
