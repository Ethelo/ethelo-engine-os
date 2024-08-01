cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Update Erlang paths for the Erlang Solutions package
set(ERLANG_ROOT /usr/lib/erlang)
set(ERLANG_LIB_DIR ${ERLANG_ROOT}/lib/erl_interface-5.5.1/lib)
set(ERLANG_INCLUDE_DIR ${ERLANG_ROOT}/lib/erl_interface-5.5.1/include
                       ${ERLANG_ROOT}/usr/include)

find_library(ERLANG_EI_LIBRARY
    NAMES ei
    PATHS
        ${ERLANG_LIB_DIR}
    NO_DEFAULT_PATH
)

if(NOT ERLANG_EI_LIBRARY)
    message(FATAL_ERROR "Could not find Erlang Interface library. Searched in ${ERLANG_LIB_DIR}")
endif()

message(STATUS "Found Erlang Interface library: ${ERLANG_EI_LIBRARY}")

include_directories(${ERLANG_INCLUDE_DIR})

# Create the ethelo_driver executable
add_executable(ethelo_driver
    driver.cpp
    engine_processor.cpp
    processor.cpp
    conversions.cpp
    communication.cpp
)

# Include Erlang headers and your project headers
target_include_directories(ethelo_driver PRIVATE
    ${ERLANG_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine
    ${CMAKE_CURRENT_SOURCE_DIR}/../../api
)

# Link against required libraries
target_link_libraries(ethelo_driver
    ethelo_api
    pthread
    ${ERLANG_EI_LIBRARY}
)

# Set compile options
target_compile_options(ethelo_driver PRIVATE
    -Wall -Wextra -Werror
    -Wno-unused-parameter
    -Wno-ignored-qualifiers
)

# If you're using any C++11 features, ensure they're enabled
target_compile_features(ethelo_driver PRIVATE cxx_std_11)
