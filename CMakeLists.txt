cmake_minimum_required(VERSION 3.15.2)

option(NO_TESTS "Do not build tests" OFF)

project(OptionParser VERSION 0.2.0 LANGUAGES CXX)

# Library
add_library(OptionParser STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OptionParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OptionParser_v2.cpp
)

set_target_properties(OptionParser PROPERTIES PUBLIC_HEADER 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OptionParser.hpp;${CMAKE_CURRENT_SOURCE_DIR}/src/OptionParser_v2.hpp"
)
target_include_directories(
    OptionParser
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_features(OptionParser PRIVATE cxx_std_20)
target_compile_options(OptionParser PRIVATE -Wall -Wextra -Wswitch)

# Install
install(TARGETS OptionParser LIBRARY PUBLIC_HEADER)
set(cmake_files_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/OptionParser)
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OptionParserConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OptionParserConfig.cmake
    INSTALL_DESTINATION ${cmake_files_install_dir}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/OptionParserConfigVersion.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/OptionParserConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/OptionParserConfigVersion.cmake
    DESTINATION ${cmake_files_install_dir} 
)

# Tests
if (NOT NO_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Examples
add_subdirectory(examples)
