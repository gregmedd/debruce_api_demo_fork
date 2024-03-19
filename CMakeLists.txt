cmake_minimum_required(VERSION 3.16)
project(api-demo VERSION 0.1.0  LANGUAGES CXX DESCRIPTION "Demo for copyable transport object with callables")

include_directories(include)

add_library(transportImpl  SHARED lib_src/TransportImpl.cpp)

add_executable(tst tst_src/tst.cpp lib_src/Transport.cpp)
target_link_libraries(tst transportImpl)


# add_library(transport1  SHARED lib_src/Transport1.cpp)