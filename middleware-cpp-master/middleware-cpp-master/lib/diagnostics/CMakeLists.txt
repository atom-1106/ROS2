# Copyright (C) Caterpillar Inc. All Rights Reserved.
project(MiddlewareDiagnosticsService)


add_library(${PROJECT_NAME}_interface INTERFACE)
target_link_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/lib
)
target_include_directories(${PROJECT_NAME}_interface SYSTEM INTERFACE
    ${PROTOBUF_INCLUDE_DIR}
)
target_include_directories(${PROJECT_NAME}_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/types
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCE_FILES
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/MiddlewareFrameworkFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CodesFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DiagnosticService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/CodeClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/CodeServer.cpp
)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_NAME}_interface
        pthread
        ${BOOST_PACKAGES}
        ${FAST_DDS_PACKAGE}
        ${PUGIXML_PACKAGE}
        protobuf::libprotobuf
        abseil::abseil
    PRIVATE
        MiddlewareFramework
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


# Copy the release required headers
set(RELEASE_OUTPUT_DIRECTORY ${MIDDLEWARE_INCLUDE_DIR}/diagnostics)
file(MAKE_DIRECTORY ${RELEASE_OUTPUT_DIRECTORY})
set(RELEASE_INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ICodeClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ICodeServer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IDiagnosticService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CodesDefines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CodesFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CodesStructures.h
)
file(COPY ${RELEASE_INCLUDE_FILES} DESTINATION ${RELEASE_OUTPUT_DIRECTORY})
