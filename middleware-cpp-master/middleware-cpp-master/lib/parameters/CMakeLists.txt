# Copyright (C) Caterpillar Inc. All Rights Reserved.
project(MiddlewareParameterService)


add_library(${PROJECT_NAME}_interface INTERFACE)
target_link_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/lib
)
target_include_directories(${PROJECT_NAME}_interface SYSTEM INTERFACE
    ${PROTOBUF_INCLUDE_DIR}
)
target_include_directories(${PROJECT_NAME}_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/types/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/types
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCE_FILES
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/MiddlewareFrameworkFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterServiceFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterEndpointConfigurationUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/ConfigBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/PublishParameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/ReadParameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/SubscribeParameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/WriteParameter.cpp
)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_NAME}_interface
        ${PUGIXML_PACKAGE}
        pthread
        ${BOOST_PACKAGES}
        ${FAST_DDS_PACKAGE}
        protobuf::libprotobuf
        abseil::abseil
    PRIVATE
        MiddlewareFramework
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


# Copy the release required headers
set(RELEASE_OUTPUT_DIRECTORY ${MIDDLEWARE_INCLUDE_DIR}/parameters)
file(MAKE_DIRECTORY ${RELEASE_OUTPUT_DIRECTORY})
set(RELEASE_INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IParameterService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IConfigBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IConfigTaker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types/interfaces/IBasicParameter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types/interfaces/IPublishParameter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types/interfaces/IReadParameter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types/interfaces/IWriteParameter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterServiceDefines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterServiceFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterServiceUnits.h
)
file(COPY ${RELEASE_INCLUDE_FILES} DESTINATION ${RELEASE_OUTPUT_DIRECTORY})
