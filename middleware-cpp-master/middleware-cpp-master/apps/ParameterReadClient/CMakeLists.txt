# Copyright (C) Caterpillar Inc. All Rights Reserved.

project(ParameterReadClient)

# External Libraries
set(LIBS ${LIBS}
    MiddlewareParameterService
    ${BOOST_PACKAGES}
    ${FAST_DDS_PACKAGE}
    ${PUGIXML_PACKAGE}
)

# Source Files
set(SOURCE_FILES ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterReadClient.cpp
)


# Target
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread ${LIBS})
# Include Inside Project files
target_include_directories(${PROJECT_NAME} PRIVATE
    # Application
    ${CMAKE_CURRENT_SOURCE_DIR}
)


set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${APPLICATIONS_BINARY_DIR})
