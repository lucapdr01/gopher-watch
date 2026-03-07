cmake_minimum_required(VERSION 3.10)
project(CStore)

set(CMAKE_CXX_STANDARD 17)

# Find the Protobuf package
find_package(Protobuf REQUIRED)

# This command tells CMake to generate .pb.h and .pb.cc from our .proto
# We point to the shared proto directory to ensure Go and C++ stay in sync
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ../proto/metrics.proto)

add_executable(cstore main.cpp ${PROTO_SRCS} ${PROTO_HDRS})

# Link the necessary libraries
target_link_libraries(cstore PUBLIC protobuf::libprotobuf)
target_include_directories(cstore PUBLIC ${CMAKE_CURRENT_BINARY_DIR})