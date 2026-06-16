# COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
cmake_minimum_required(VERSION 3.28)

project(MiddlewareTest)


# Include files from outside project
link_directories( SYSTEM
    ${ARTIFACTS_DIR}/lib
)

# Include files from outside project
include_directories( SYSTEM
    ${ARTIFACTS_DIR}/include
    ${PROTOBUF_INCLUDE_DIR}
)
# Include files from inside project
include_directories(
    # Utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities
    # Fakes
    ${CMAKE_CURRENT_SOURCE_DIR}/fakes/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/fakes/parameters
    # Framework (common also provides formerly-supporting headers)
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/common
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/interfaces
    ${MIDDLEWARE_ROOT}/idl/topics/message
    ${MIDDLEWARE_ROOT}/idl/topics/requests
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/eprosima/interfaces
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/eprosima
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/client
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/common
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/common/interfaces
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/message
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/server/interfaces
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds/server
    ${MIDDLEWARE_LIB_ROOT_DIR}/framework/dds
    # Parameters
    ${MIDDLEWARE_LIB_ROOT_DIR}/parameters/interfaces
    ${MIDDLEWARE_LIB_ROOT_DIR}/parameters
    ${MIDDLEWARE_LIB_ROOT_DIR}/parameters/types/interfaces
    ${MIDDLEWARE_LIB_ROOT_DIR}/parameters/types
    # Diagnostics
    ${MIDDLEWARE_LIB_ROOT_DIR}/diagnostics
    ${MIDDLEWARE_LIB_ROOT_DIR}/diagnostics/interfaces
    ${MIDDLEWARE_LIB_ROOT_DIR}/diagnostics/types
    # Test
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/common
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/parameters
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Sources
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/MiddlewareTests.cpp
    # Fakes
    ${CMAKE_CURRENT_SOURCE_DIR}/fakes/framework/FakeMiddlewareFrameworkFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fakes/parameters/FakeParameterServiceFactory.cpp
    # Framework Tests
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ServicesCommonTest.cpp # Must be here
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareSystemChecks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/CommonFrameworkTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareFrameworkMessageReaderTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareFrameworkMessageWriterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareFrameworkMessageParticipantTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareFrameworkMiddlewareClientTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareFrameworkMiddlewareServerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/MiddlewareFrameworkServerRequestHandlerTest.cpp
    # Parameters Tests
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/ParameterEndpointConfigurationUtilitiesTest.cpp # Must be here before other parameter tests
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/PublishParameterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/ReadParameterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/SubscribeParameterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/WriteParameterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/ConfigBuilderTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/ParameterServiceTest.cpp
    ## Diagnostics Tests
    ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/MiddlewareCodeServerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/MiddlewareCodeClientTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/DiagnosticServiceTest.cpp
)

# Target
#add_compile_options(--coverage -fprofile-arcs -ftest-coverage)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
        MiddlewareFramework
        ${GTEST_PACKAGES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_BINARY_DIR})
