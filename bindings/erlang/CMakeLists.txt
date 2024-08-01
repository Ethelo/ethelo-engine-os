set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(ethelo_driver driver.cpp engine_processor.cpp processor.cpp conversions.cpp communication.cpp)
target_include_directories(ethelo_driver PRIVATE /usr/lib/erlang/usr/include)
target_link_libraries(ethelo_driver ethelo_api pthread /usr/lib/erlang/usr/lib/liberl_interface.a /usr/lib/erlang/usr/lib/libei.a)
