file(GLOB TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# A semi-colon separated list of test sources that should not be automatically built with doctest
set(SPECIAL_TESTS)

if(CMAKE_VERSION VERSION_LESS 2.8)
  # Portability test uses the `TARGET_FILE_DIR` generator expression which is available from CMake 2.8.
  set(SKIP_PORTABILITY_TEST ON)
endif()

# Build all of the non-special tests
foreach(TEST_SOURCE ${TESTS})

  string(REPLACE ".cpp" "" TEST_TARGET "${TEST_SOURCE}")
  set(TEST_TARGET "test_${TEST_TARGET}")

  # Check to see if our target is listed in "SPECIAL_TESTS"
  list(FIND SPECIAL_TESTS "${TEST_SOURCE}" IS_SPECIAL_TEST)

  if(IS_SPECIAL_TEST EQUAL -1)

    add_executable(${TEST_TARGET} ${TEST_SOURCE})
    target_link_libraries(${TEST_TARGET} ${SER20_THREAD_LIBS})
    add_test(NAME "${TEST_TARGET}" COMMAND "${TEST_TARGET}")

  endif()

endforeach()

# Add the valgrind target
if(NOT MSVC)
  add_custom_target(valgrind
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/run_valgrind.sh")

  # Add the coverage target
  add_custom_target(coverage)
  add_custom_command(TARGET coverage
    COMMAND "${CMAKE_SOURCE_DIR}/scripts/updatecoverage.sh" ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/coverage")

  # add tests to coverage
  foreach(TEST_SOURCE ${TESTS})
    string(REPLACE ".cpp" "" COVERAGE_TARGET "${TEST_SOURCE}")
    set(COVERAGE_TARGET "coverage_${COVERAGE_TARGET}")

    # Check to see if our target is listed in "SPECIAL_TESTS"
    list(FIND SPECIAL_TESTS "${TEST_SOURCE}" IS_SPECIAL_TEST)

    if(IS_SPECIAL_TEST EQUAL -1)
      add_dependencies(coverage ${COVERAGE_TARGET})

      add_executable(${COVERAGE_TARGET} EXCLUDE_FROM_ALL ${TEST_SOURCE})
      set_target_properties(${COVERAGE_TARGET} PROPERTIES COMPILE_FLAGS "-coverage")
      set_target_properties(${COVERAGE_TARGET} PROPERTIES LINK_FLAGS "-coverage")
      set_target_properties(${COVERAGE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/coverage")
      target_link_libraries(${COVERAGE_TARGET} ${SER20_THREAD_LIBS})
    endif()
  endforeach()
endif()

if(Boost_FOUND)
  add_subdirectory(boost)
endif()

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
  add_test(NAME test_cmake_config_module COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake-config-module.cmake)
endif()
