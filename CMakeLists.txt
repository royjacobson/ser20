cmake_minimum_required(VERSION 3.20)

project(cereal LANGUAGES CXX VERSION 1.4.0)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CEREAL_MASTER_PROJECT ON)
endif()


if(APPLE)
    option(SKIP_PORTABILITY_TEST "Skip portability (32 bit) tests" ON)
endif()

option(BUILD_DOC "Build documentation" ${CEREAL_MASTER_PROJECT})
option(BUILD_SANDBOX "Build sandbox examples" ${CEREAL_MASTER_PROJECT})
option(BUILD_TESTS "Build tests" ${CEREAL_MASTER_PROJECT})
option(SKIP_PERFORMANCE_COMPARISON "Skip building performance sandbox comparison (requires boost)" ON)

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD STREQUAL "98")
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CEREAL_SOURCES "src/cereal.cpp")
add_library(cereal STATIC ${CEREAL_SOURCES})
add_library(cereal::cereal ALIAS cereal)

target_include_directories(cereal SYSTEM PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


if(MSVC)
    target_compile_options(cereal PRIVATE /bigobj /W3 /WX)
else()
    target_compile_options(cereal PRIVATE -Wall -Wextra -pedantic -Wshadow -Wold-style-cast)
    option(WITH_WERROR "Compile with '-Werror' C++ compiler flag" ON)
    if(WITH_WERROR)
        target_compile_options(cereal PRIVATE -Werror)
    endif()

    option(CLANG_USE_LIBCPP "Use libc++ for clang compilation" OFF)
    if(APPLE OR CLANG_USE_LIBCPP)
        message(STATUS "Use libc++")
        target_compile_options(cereal PUBLIC -stdlib=libc++)
        target_link_options(cereal PUBLIC -stdlib=libc++ -lc++abi)
    endif()
endif()


set(CEREAL_THREAD_LIBS)
if(UNIX)
    option(THREAD_SAFE "Use mutexes to ensure thread safety" OFF)
    if(THREAD_SAFE)
        message(STATUS "Use mutexes")
        target_compile_definitions(cereal PUBLIC CEREAL_THREAD_SAFE=1)
        set(CEREAL_THREAD_LIBS pthread)
    endif()
endif()

list(APPEND CEREAL_THREAD_LIBS cereal::cereal)

target_compile_features(cereal INTERFACE cxx_std_20)


option(CEREAL_INSTALL "Generate the install target" ${CEREAL_MASTER_PROJECT})
if(CEREAL_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS cereal EXPORT ${PROJECT_NAME}Targets)
    install(DIRECTORY include/cereal DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    set(configFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
    set(versionFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
    set(configInstallDestination ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        ${configFile}
        INSTALL_DESTINATION ${configInstallDestination}
    )
    write_basic_package_version_file(
        ${versionFile}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES ${configFile} ${versionFile} DESTINATION ${configInstallDestination})
    install(
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE "cereal::"
        DESTINATION ${configInstallDestination}
    )
endif()

if(NOT SKIP_PERFORMANCE_COMPARISON)
    # Boost serialization for performance sandbox
    find_package(Boost REQUIRED COMPONENTS serialization)
endif()


if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(unittests)
endif()


if(BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif()


if(BUILD_DOC)
    add_subdirectory(doc)
endif()
