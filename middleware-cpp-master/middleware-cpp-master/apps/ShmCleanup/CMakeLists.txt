# Copyright (C) Caterpillar Inc. All Rights Reserved.

project(ShmCleanup)

# External Libraries
set(LIBS ${LIBS}
    MiddlewareFramework
)

# Source Files
set(SOURCE_FILES ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)


# Target
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread ${LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${MIDDLEWARE_ROOT}/lib/framework/common
)


set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${APPLICATIONS_BINARY_DIR})
