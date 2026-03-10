# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Find Google Test
find_package(GTest REQUIRED)

# Unit tests
set(UNIT_TEST_SOURCES
    unit/test_audio_dsp.cpp
    unit/test_midi_messages.cpp
    unit/test_style_engine.cpp
    unit/test_effects.cpp
)

add_executable(maestro_unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(maestro_unit_tests PRIVATE
    maestro_core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME UnitTests COMMAND maestro_unit_tests)

# Integration tests
set(INTEGRATION_TEST_SOURCES
    integration/test_engine_integration.cpp
)

add_executable(maestro_integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(maestro_integration_tests PRIVATE
    maestro_core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME IntegrationTests COMMAND maestro_integration_tests)

# Benchmarks (optional, requires Google Benchmark)
find_package(benchmark QUIET)
if(benchmark_FOUND)
    set(BENCHMARK_SOURCES
        benchmark/benchmark_audio.cpp
        benchmark/benchmark_midi.cpp
        benchmark/benchmark_dsp.cpp
    )

    add_executable(maestro_benchmarks ${BENCHMARK_SOURCES})
    target_link_libraries(maestro_benchmarks PRIVATE
        maestro_core
        benchmark::benchmark
    )
endif()
