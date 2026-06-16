# Copyright (C) Caterpillar Inc. All Rights Reserved.
cmake_minimum_required(VERSION 3.20)

project(MiddlewareFramework_protobuf)

get_filename_component(HERMES_PROTO_DIR "source" ABSOLUTE)


# Hermes gRPC proto Definitions
set(HERMES_GRPC_PROTO_FILES
    hermes_transaction.proto
)
GenerateGRPCProtobufFilesLibraries("${HERMES_GRPC_PROTO_FILES}" "${PROTOBUF_OUTPUT_DIRECTORY}" "${HERMES_PROTO_DIR}" "")
add_library(${PROJECT_NAME}_transaction OBJECT ${ALL_GENERATED_PROTO_FILES})
target_link_libraries(${PROJECT_NAME}_transaction PRIVATE ServiceDependencies_grpc)

unset(ALL_GENERATED_PROTO_FILES)

# Final Combined Library
add_library(${PROJECT_NAME} STATIC
    $<TARGET_OBJECTS:${PROJECT_NAME}_transaction>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
