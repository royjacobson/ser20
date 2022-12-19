file(GLOB TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Build all of the non-special tests
foreach(TEST_SOURCE ${TESTS})
  message(STATUS ${TEST_SOURCE})

  string(REPLACE ".cpp" "" TEST_TARGET "${TEST_SOURCE}")
  set(TEST_TARGET "test_${TEST_TARGET}")

  add_executable(${TEST_TARGET} ${TEST_SOURCE})
  target_link_libraries(${TEST_TARGET} ${SER20_THREAD_LIBS})
  add_test(NAME "${TEST_TARGET}" COMMAND "${TEST_TARGET}")

endforeach()

if(NOT MSVC)
  # add tests to coverage
  foreach(TEST_SOURCE ${TESTS})
    string(REPLACE ".cpp" "" COVERAGE_TARGET "${TEST_SOURCE}")
    set(COVERAGE_TARGET "coverage_${COVERAGE_TARGET}")

    add_dependencies(coverage ${COVERAGE_TARGET})

    add_executable(${COVERAGE_TARGET} EXCLUDE_FROM_ALL ${TEST_SOURCE})
    set_target_properties(${COVERAGE_TARGET} PROPERTIES COMPILE_FLAGS "-coverage")
    set_target_properties(${COVERAGE_TARGET} PROPERTIES LINK_FLAGS "-coverage")
    set_target_properties(${COVERAGE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/coverage")
    target_link_libraries(${COVERAGE_TARGET} ${SER20_THREAD_LIBS})
  endforeach()
endif()
