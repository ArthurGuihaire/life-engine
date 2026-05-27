cmake_minimum_required(VERSION 3.16)
project(life_engine LANGUAGES CXX)

# find needed sfml libraries
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# enable compile command exporting for lsp support
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# set to release build by default
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# enable link time optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if(supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported: ${error}")
endif()

# find source and header files and add them
file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# dynamic link sfml
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)
