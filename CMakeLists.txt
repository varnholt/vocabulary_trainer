cmake_minimum_required(VERSION 3.22)
project(
  vocabulary_trainer
  VERSION 2.0.0
  LANGUAGES CXX)

add_library(vocabulary_core src/training.cpp)
target_include_directories(vocabulary_core PUBLIC src)
target_compile_features(vocabulary_core PUBLIC cxx_std_17)
set_target_properties(vocabulary_core PROPERTIES CXX_EXTENSIONS OFF)
if(MSVC)
  target_compile_options(
    vocabulary_core
    PUBLIC /utf-8
    PRIVATE /W4 /permissive-)
else()
  target_compile_options(vocabulary_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EMSCRIPTEN)
  add_executable(vocabulary src/web.cpp)
  target_link_libraries(vocabulary PRIVATE vocabulary_core)
  set_target_properties(
    vocabulary PROPERTIES SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY
                                       "${CMAKE_BINARY_DIR}/web")
  target_link_options(
    vocabulary PRIVATE -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web -sFILESYSTEM=0
    "-sEXPORTED_RUNTIME_METHODS=['cwrap']")
  add_custom_target(
    web_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/web"
            "${CMAKE_BINARY_DIR}/web"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/testdata"
            "${CMAKE_BINARY_DIR}/web/samples"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/licenses"
            "${CMAKE_BINARY_DIR}/web/licenses"
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/LICENSE"
            "${PROJECT_SOURCE_DIR}/THIRD_PARTY.md" "${CMAKE_BINARY_DIR}/web/"
    DEPENDS vocabulary)
  install(DIRECTORY "${CMAKE_BINARY_DIR}/web/" DESTINATION .)
endif()

include(CTest)
if(BUILD_TESTING AND NOT EMSCRIPTEN)
  add_executable(training_tests tests/training_tests.cpp)
  target_link_libraries(training_tests PRIVATE vocabulary_core)
  add_test(NAME training_regressions COMMAND training_tests)
endif()
