# CMakeLists.txt
cmake_minimum_required(VERSION 3.15.2)

project(OptionParser VERSION 0.0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME} 
    INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS ${PROJECT_NAME} 
    EXPORT "${PROJECT_NAME}Targets"
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
)


install(EXPORT "${PROJECT_NAME}Targets" DESTINATION "lib/cmake/${PROJECT_NAME}")
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "lib/cmake/${PROJECT_NAME}")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)



