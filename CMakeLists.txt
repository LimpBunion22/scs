cmake_minimum_required(VERSION 3.16)

project(space_combat_strategy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(scs_core STATIC
    src/gameplay/time_scale_policy.cpp
    src/presentation/prediction.cpp
    src/presentation/tactical_snapshot.cpp
    src/simulation/contact_tracker.cpp
    src/simulation/replay.cpp
    src/simulation/scenario.cpp
    src/simulation/simulation.cpp
)

target_include_directories(scs_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(scs_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(scs_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(scs_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(scs_demo
    src/app/main.cpp
)

target_link_libraries(scs_demo PRIVATE scs_core)

enable_testing()

add_executable(scs_core_tests
    tests/simulation/deterministic_replay_tests.cpp
)

target_link_libraries(scs_core_tests PRIVATE scs_core)

add_test(NAME scs_core_tests COMMAND scs_core_tests)

add_executable(scs_contact_tests
    tests/simulation/contact_tests.cpp
)

target_link_libraries(scs_contact_tests PRIVATE scs_core)

add_test(NAME scs_contact_tests COMMAND scs_contact_tests)

add_executable(scs_time_scale_policy_tests
    tests/gameplay/time_scale_policy_tests.cpp
)

target_link_libraries(scs_time_scale_policy_tests PRIVATE scs_core)

add_test(NAME scs_time_scale_policy_tests COMMAND scs_time_scale_policy_tests)

add_executable(scs_presentation_tests
    tests/presentation/tactical_snapshot_tests.cpp
)

target_link_libraries(scs_presentation_tests PRIVATE scs_core)

add_test(NAME scs_presentation_tests COMMAND scs_presentation_tests)
