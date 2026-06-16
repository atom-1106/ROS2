# Copyright (C) Caterpillar Inc. All Rights Reserved.

project(DiscoveryServer)

# External Libraries
set(LIBS ${LIBS}
    ${BOOST_PACKAGES}
    ${FAST_DDS_PACKAGE}
    ${PUGIXML_PACKAGE}
)

# Include Inside Project files
include_directories(
    # Application
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# Source Files
set(SOURCE_FILES ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)


# Target
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} pthread ${LIBS})


set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${APPLICATIONS_BINARY_DIR})
