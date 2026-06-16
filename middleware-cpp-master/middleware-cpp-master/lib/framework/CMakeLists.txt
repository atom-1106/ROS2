# Copyright (C) Caterpillar Inc. All Rights Reserved.
project(MiddlewareFramework)

set(FRAMEWORK_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(dds)
## [TODO] Needs rework
#add_subdirectory(grpc)


# MiddlewareFramework is a static library that absorbs:
#   - Framework common sources (including formerly-separate supporting sources)
#   - Topic, RPC interface, implementation and protobuf OBJECT sub-libraries
add_library(${PROJECT_NAME} STATIC
    # Framework common sources
    ${CMAKE_CURRENT_SOURCE_DIR}/common/ShmCleanup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/TestUtilities.cpp
    # Supporting sources (moved from lib/supporting into lib/framework/common)
    ${CMAKE_CURRENT_SOURCE_DIR}/common/SystemUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/SingleThreadedUsageValidator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/ShmemCleanup_StaticMembers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/TestUtilities_StaticMembers.cpp
    # Topic, RPC interface, implementation and protobuf objects statically linked in
    $<TARGET_OBJECTS:MiddlewareFrameworkMessageTopic>
    $<TARGET_OBJECTS:MiddlewareFrameworkRequestInterface>
    $<TARGET_OBJECTS:${PROJECT_NAME}_impl>
    $<TARGET_OBJECTS:MiddlewareProtobuf>
)
target_include_directories(${PROJECT_NAME} PUBLIC
    # Framework Common (also provides formerly-supporting headers)
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        pthread
        ${BOOST_PACKAGES}
        ${FAST_DDS_PACKAGE}
        fastcdr
        protobuf::libprotobuf
        abseil::abseil
        ${PUGIXML_PACKAGE}
        # Interface libraries provide include directories from sub-libraries
        MiddlewareFramework_interface
        MiddlewareFrameworkMessageTopic_interface
        MiddlewareFrameworkRequestInterface_interface
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
