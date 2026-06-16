# Copyright (C) Caterpillar Inc. All Rights Reserved.

project(middleware-parameters-tester)

# External Libraries
set(LIBS ${LIBS}
    MiddlewareParameterService
    ${BOOST_PACKAGES}
    ${FAST_DDS_PACKAGE}
)

# Source Files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Actions.cpp
)

# Target
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread ${LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${APPLICATIONS_BINARY_DIR})
