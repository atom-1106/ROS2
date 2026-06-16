# COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
cmake_minimum_required(VERSION 3.20)

project(MiddlewareFramework_grpc)

cmake_policy(SET CMP0022 NEW) # Needed for proper modern CMake link library propogation

add_library(${PROJECT_NAME}_interface INTERFACE)
target_link_directories(${PROJECT_NAME}_interface INTERFACE 
    ${ARTIFACTS_DIR}/lib
)
target_include_directories(${PROJECT_NAME}_interface SYSTEM INTERFACE
    ${PROTOBUF_OUTPUT_DIRECTORY}
)


target_include_directories(${PROJECT_NAME}_interface INTERFACE
    # Root Common
    ${SERVICES_COMMON_DIR}/interfaces
    ${SERVICES_COMMON_DIR}
    ${FRAMEWORK_ROOT_DIR}
    ${FRAMEWORK_COMMON_DIR}
    ${FRAMEWORK_COMMON_DIR}/interfaces
    ${FRAMEWORK_INTERFACES_DIR}
    # GRPC Common
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    # Source
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/client
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/server/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCE_FILES ${SOURCE_FILES}
    ${SERVICES_COMMON_DIR}/SystemUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HermesMiddlewareFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StubCreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/HermesGrpcTools.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/HermesCallbackService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/HermesGrpcServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/client/HermesGrpcClient.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        ${PROJECT_NAME}_interface
        pthread
    PRIVATE
        MiddlewareFramework_protobuf
        MiddlewareFramework_dds
        ServiceDependencies_grpc
        ${FAST_DDS_PACKAGE}
        ${BOOST_PACKAGES}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
