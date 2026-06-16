# COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
cmake_minimum_required(VERSION 3.28)

project(IntegrationTestApp)


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
    ${CMAKE_CURRENT_SOURCE_DIR}/instances
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Sources
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterServiceIntegration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterServiceClientServerIntegration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/instances/ParameterServiceOwnerInstance.cpp
)

# Target
#add_compile_options(--coverage -fprofile-arcs -ftest-coverage)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
        MiddlewareParameterService
        MiddlewareDiagnosticsService
        ${FAST_DDS_PACKAGE}
        ${GTEST_PACKAGES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_BINARY_DIR})

