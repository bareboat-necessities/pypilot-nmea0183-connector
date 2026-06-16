cmake_minimum_required(VERSION 3.16)
project(pypilot_nmea0183_connector LANGUAGES CXX)

option(PYPILOT_NMEA0183_CONNECTOR_BUILD_TESTS "Build tests" ON)
option(PYPILOT_NMEA0183_CONNECTOR_BUILD_EXAMPLES "Build examples" ON)

set(PYPILOT_DATA_MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../pypilot-data-model/src" CACHE PATH "Path to pypilot-data-model src include dir")
set(PYPILOT_ALGORITHMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../pypilot-algorithms/src" CACHE PATH "Path to pypilot-algorithms src include dir")

add_library(pypilot_nmea0183_connector INTERFACE)
target_include_directories(pypilot_nmea0183_connector INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${PYPILOT_DATA_MODEL_DIR}
  ${PYPILOT_ALGORITHMS_DIR}
)
target_compile_features(pypilot_nmea0183_connector INTERFACE cxx_std_11)

if(PYPILOT_NMEA0183_CONNECTOR_BUILD_EXAMPLES)
  add_executable(parse_nmea_file examples/linux/parse_nmea_file.cpp)
  target_link_libraries(parse_nmea_file PRIVATE pypilot_nmea0183_connector)

  add_executable(format_heading examples/linux/format_heading.cpp)
  target_link_libraries(format_heading PRIVATE pypilot_nmea0183_connector)
endif()

if(PYPILOT_NMEA0183_CONNECTOR_BUILD_TESTS)
  enable_testing()
  foreach(test_name
      test_checksum
      test_stream_parser
      test_parse_helpers
      test_apply_gps
      test_apply_wind
      test_apply_heading
      test_format_sentence)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE pypilot_nmea0183_connector)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
endif()
