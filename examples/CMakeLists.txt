cmake_minimum_required(VERSION 3.15.2)

add_executable(gcc_command_line_example ${CMAKE_CURRENT_SOURCE_DIR}/gcc_command_line_example.cpp)
target_link_libraries(gcc_command_line_example PRIVATE OptionParser)

add_executable(optionparser_v2_help_text ${CMAKE_CURRENT_SOURCE_DIR}/optionparser_v2_help_text.cpp)
target_link_libraries(optionparser_v2_help_text PRIVATE OptionParser)
