cmake_minimum_required(VERSION 3.15.2)

add_executable(gcc_command_line_example ${CMAKE_CURRENT_SOURCE_DIR}/gcc_command_line_example.cpp)
target_link_libraries(gcc_command_line_example PRIVATE OptionParser)
