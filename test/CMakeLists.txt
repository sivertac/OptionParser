# CMakeLists.txt
cmake_minimum_required(VERSION 3.15.2)

# So that GoogleTest does not get installed with this project
set(INSTALL_GTEST OFF)

# Download and unpack googletest at configure time
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

get_target_property(main-sources OptionParser SOURCES)
list(FILTER main-sources EXCLUDE REGEX ".*main.cpp$")
get_target_property(main-includes OptionParser INCLUDE_DIRECTORIES)
get_target_property(main-libraries OptionParser LINK_LIBRARIES)

add_library(test-main-asan ${main-sources})
target_include_directories(test-main-asan PUBLIC ${main-includes})
target_compile_features(test-main-asan PUBLIC cxx_std_20)
target_compile_options(test-main-asan PUBLIC -g -Wall -Wextra -Wswitch)
target_compile_options(test-main-asan PUBLIC -fsanitize=address)
target_link_libraries(test-main-asan PUBLIC -fsanitize=address)
#target_link_libraries(test-main-asan INTERFACE ${main-libraries})

add_library(test-main-tsan ${main-sources})
target_include_directories(test-main-tsan PUBLIC ${main-includes})
target_compile_features(test-main-tsan PUBLIC cxx_std_20)
target_compile_options(test-main-tsan PUBLIC -g -Wall -Wextra -Wswitch)
target_compile_options(test-main-tsan PUBLIC -fsanitize=thread)
target_link_libraries(test-main-tsan PUBLIC -fsanitize=thread)
#target_link_libraries(test-main-tsan INTERFACE ${main-libraries})

function(create_test test_name test_sources)
    add_executable(${test_name} ${test_sources})
    target_link_libraries(${test_name} PUBLIC gtest_main)
    gtest_discover_tests(${test_name})
endfunction()

function(create_xsan_test test_name test_sources)
    create_test(${test_name}-asan ${test_sources})
    target_link_libraries(${test_name}-asan PUBLIC test-main-asan)
    create_test(${test_name}-tsan ${test_sources})
    target_link_libraries(${test_name}-tsan PUBLIC test-main-tsan)
endfunction()

create_xsan_test(test_OptionParser ${CMAKE_CURRENT_SOURCE_DIR}/test_OptionParser.cpp)
