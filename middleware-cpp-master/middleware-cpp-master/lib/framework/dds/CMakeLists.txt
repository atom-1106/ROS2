# Copyright (C) Caterpillar Inc. All Rights Reserved.
project(MiddlewareFramework)

set(TOPIC_INCLUDE_DIRECTORIES
    ${MIDDLEWARE_ROOT}/idl/topics/message
    ${MIDDLEWARE_ROOT}/idl/topics/requests
)

add_library(${PROJECT_NAME}_interface INTERFACE)
target_link_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/lib
)
target_include_directories(${PROJECT_NAME}_interface INTERFACE
    # Framework Common (includes formerly-supporting headers)
    ${FRAMEWORK_ROOT_DIR}/common
    ${FRAMEWORK_ROOT_DIR}/interfaces
    # DDS Common
    ${CMAKE_CURRENT_SOURCE_DIR}/common/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    # DDS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima
    ${CMAKE_CURRENT_SOURCE_DIR}/message
    ${CMAKE_CURRENT_SOURCE_DIR}/client
    ${CMAKE_CURRENT_SOURCE_DIR}/server/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/spy
    # Topics
    ${TOPIC_INCLUDE_DIRECTORIES}
)

add_library(${PROJECT_NAME}_impl OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/common/ThreadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/DataReaderStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/DataWriterStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/MiddlewareDataReaderListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/MiddlewareDataWriterListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/DomainParticipantMessageStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/SmartDdsPointers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/ClientRpcStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eprosima/ServerRpcStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/message/MessageWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/message/MessageReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/message/MessageParticipant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/MiddlewareServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/ServerRequestHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/client/MiddlewareClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spy/MiddlewareSpy.cpp
)
target_link_libraries(${PROJECT_NAME}_impl
    PUBLIC
        ${PROJECT_NAME}_interface
        pthread
        ${FAST_DDS_PACKAGE}
        protobuf::libprotobuf
        ${BOOST_PACKAGES}
)
set_property(TARGET ${PROJECT_NAME}_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

# Copy the release required headers
set(RELEASE_OUTPUT_DIRECTORY ${MIDDLEWARE_INCLUDE_DIR}/framework)
file(MAKE_DIRECTORY ${RELEASE_OUTPUT_DIRECTORY})
set(RELEASE_INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/spy/MiddlewareSpy.h
)
file(COPY ${RELEASE_INCLUDE_FILES} DESTINATION ${RELEASE_OUTPUT_DIRECTORY})
