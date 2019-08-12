# CMakeLists.txt
cmake_minimum_required(VERSION 3.15.2)

project(OptionParser_test)   

find_package(doctest REQUIRED)
include_directories(${doctest_INCLUDE_DIR})

enable_testing()

add_library(test_main OBJECT test_main.cpp)

#target_include_directories(test_main PRIVATE ${DOCTEST_INCLUDE_PATH})

add_executable(test1 test1.cpp $<TARGET_OBJECTS:test_main>)
#target_include_directories(test1 PRIVATE ${DOCTEST_INCLUDE_PATH})
add_test(NAME test1 COMMAND test1)

add_executable(test2 test2.cpp $<TARGET_OBJECTS:test_main>)
#target_include_directories(test2 PRIVATE ${DOCTEST_INCLUDE_PATH})
add_test(NAME test2 COMMAND test2)


