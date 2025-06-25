cmake_minimum_required(VERSION 3.20)

project(ser20 LANGUAGES CXX VERSION 1.4.0)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SER20_MASTER_PROJECT ON)
endif()

option(BUILD_DOC "Build documentation" ${SER20_MASTER_PROJECT})
option(BUILD_SANDBOX "Build sandbox examples" ${SER20_MASTER_PROJECT})
option(BUILD_TESTS "Build tests" ${SER20_MASTER_PROJECT})
option(SKIP_PERFORMANCE_COMPARISON "Skip building performance sandbox comparison (requires boost)" ON)

set(SER20_SOURCES "src/ser20.cpp" "src/polymorphic_impl.cpp")
add_library(ser20 ${SER20_SOURCES})
add_library(ser20::ser20 ALIAS ser20)

target_include_directories(ser20 SYSTEM PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ser20 PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(ser20 PRIVATE /bigobj /W3 /WX)
else()
    target_compile_options(ser20 PRIVATE -Wall -Wextra -pedantic -Wshadow -Wold-style-cast)
    option(WITH_WERROR "Compile with '-Werror' C++ compiler flag" ON)

    if(WITH_WERROR)
        target_compile_options(ser20 PRIVATE -Werror)
    endif()

    option(CLANG_USE_LIBCPP "Use libc++ for clang compilation" OFF)

    if(APPLE OR CLANG_USE_LIBCPP)
        message(STATUS "Use libc++")
        target_compile_options(ser20 PUBLIC -stdlib=libc++)
        target_link_options(ser20 PUBLIC -stdlib=libc++ -lc++abi)
    endif()
endif()

set(SER20_THREAD_LIBS)

if(UNIX)
    option(THREAD_SAFE "Use mutexes to ensure thread safety" OFF)

    if(THREAD_SAFE)
        message(STATUS "Use mutexes")
        target_compile_definitions(ser20 PUBLIC SER20_THREAD_SAFE=1)
        set(SER20_THREAD_LIBS pthread)
    endif()
endif()

list(APPEND SER20_THREAD_LIBS ser20::ser20)

target_compile_features(ser20 INTERFACE cxx_std_20)

option(SER20_INSTALL "Generate the install target" ${SER20_MASTER_PROJECT})

if(SER20_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS ser20 EXPORT ${PROJECT_NAME}Targets)
    install(DIRECTORY include/ser20 DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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
        NAMESPACE "ser20::"
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
