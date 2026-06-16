# COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.

project(middleware-framework-grpc)
# Set protobuf directory
set(PROTOBUF_OUTPUT_DIRECTORY ${INFO_SERVICES_PROTOBUF_OUTPUT_ROOT}/${PROJECT_NAME})
file(MAKE_DIRECTORY ${PROTOBUF_OUTPUT_DIRECTORY})


add_subdirectory(proto)
add_subdirectory(source)

