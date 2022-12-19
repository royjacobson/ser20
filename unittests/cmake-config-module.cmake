if(CMAKE_VERSION LESS 3.0)
  message(FATAL_ERROR "Ser20 can't be installed with CMake < 3.0")
endif()

get_filename_component(BINARY_DIR ${CMAKE_BINARY_DIR}/build ABSOLUTE)
get_filename_component(INSTALL_DIR ${CMAKE_BINARY_DIR}/out ABSOLUTE)

# cmake configure step for ser20
file(MAKE_DIRECTORY ${BINARY_DIR}/ser20)
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -DBUILD_DOC=OFF
    -DBUILD_SANDBOX=OFF
    -DBUILD_TESTS=OFF
    -DSKIP_PERFORMANCE_COMPARISON=ON
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
  WORKING_DIRECTORY ${BINARY_DIR}/ser20
  RESULT_VARIABLE result
)
if(result)
  message(FATAL_ERROR "Ser20 cmake configure-step failed")
endif()

# cmake install ser20
execute_process(
  COMMAND ${CMAKE_COMMAND}
    --build ${BINARY_DIR}/ser20
    --target install
  RESULT_VARIABLE result
)
if(result)
  message(FATAL_ERROR "Ser20 cmake install-step failed")
endif()

# create test project sources
file(WRITE ${BINARY_DIR}/test_source/CMakeLists.txt "
  cmake_minimum_required(VERSION ${CMAKE_VERSION})
  project(ser20-test-config-module)
  if(NOT MSVC)
      if(CMAKE_VERSION VERSION_LESS 3.1)
          set(CMAKE_CXX_FLAGS \"-std=c++11 \${CMAKE_CXX_FLAGS}\")
      else()
          set(CMAKE_CXX_STANDARD 20)
          set(CMAKE_CXX_STANDARD_REQUIRED ON)
      endif()
  endif()
  find_package(ser20 REQUIRED)
  add_executable(ser20-test-config-module main.cpp)
  target_link_libraries(ser20-test-config-module ser20::ser20)
  enable_testing()
  add_test(NAME test-ser20-test-config-module COMMAND ser20-test-config-module)
")

file(WRITE ${BINARY_DIR}/test_source/main.cpp "
  #include <ser20/archives/binary.hpp>
  #include <sstream>
  #include <cstdlib>
  struct MyData
  {
    int x = 0, y = 0, z = 0;
    void set() { x = 1; y = 2; z = 3; }
    bool is_set() const { return x == 1 && y == 2 && z == 3; }

    // This method lets ser20 know which data members to serialize
    template<class Archive>
    void serialize(Archive & archive)
    {
      archive( x, y, z ); // serialize things by passing them to the archive
    }
  };
  int main()
  {
    std::stringstream ss; // any stream can be used

    {
      ser20::BinaryOutputArchive oarchive(ss); // Create an output archive

      MyData m1, m2, m3;
      m1.set();
      m2.set();
      m3.set();
      oarchive(m1, m2, m3); // Write the data to the archive
    }

    {
      ser20::BinaryInputArchive iarchive(ss); // Create an input archive

      MyData m1, m2, m3;
      iarchive(m1, m2, m3); // Read the data from the archive

      return (m1.is_set() && m2.is_set() && m3.is_set())
      ? EXIT_SUCCESS : EXIT_FAILURE;
    }
  }"
)

file(MAKE_DIRECTORY ${BINARY_DIR}/test)
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR}
    ${BINARY_DIR}/test_source
  WORKING_DIRECTORY ${BINARY_DIR}/test
  RESULT_VARIABLE result
)
if(result)
  message(FATAL_ERROR "Test cmake configure-step failed")
endif()

# cmake install ser20
execute_process(
  COMMAND ${CMAKE_COMMAND}
    --build ${BINARY_DIR}/test
  RESULT_VARIABLE result
)
if(result)
  message(FATAL_ERROR "Test cmake build-step failed")
endif()

execute_process(
  COMMAND ${CMAKE_CTEST_COMMAND}
  WORKING_DIRECTORY ${BINARY_DIR}/test
  RESULT_VARIABLE result
)

if(result)
  message(FATAL_ERROR "Test run failed")
endif()
