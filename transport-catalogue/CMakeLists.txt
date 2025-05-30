cmake_minimum_required (VERSION 3.18)

project (TransportCatalogue CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS svg.proto map_renderer.proto graph.proto transport_router.proto transport_catalogue.proto)

set(CATALOGUE_FILES "main.cpp" "transport_catalogue.proto" "transport_catalogue.cpp" "json.h" "json.cpp" "svg.h" "svg.cpp" "domain.h" "domain.cpp" "json_reader.h" "json_reader.cpp" "request_handler.h" "request_handler.cpp" "map_renderer.h"  "map_renderer.cpp" "json_builder.h" "json_builder.cpp" "graph.h" "ranges.h" "router.h" "transport_router.h" "transport_router.cpp" "serialization.cpp" "serialization.h")
add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${CATALOGUE_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#string(REPLACE "libprotobuf.lib" "libprotobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
#string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY_RELEASE}>" Threads::Threads)