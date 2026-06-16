# COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
project(MiddlewareFrameworkRequestInterface)

add_library(${PROJECT_NAME}_interface INTERFACE)

target_link_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/lib
)

target_include_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Note: Not exactly sure why we need this object library layer. Will revisit later.
# Add requests RPC interface
add_library(${PROJECT_NAME} OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/requestsCdrAux.ipp
    ${CMAKE_CURRENT_SOURCE_DIR}/requestsClient.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/requestsPubSubTypes.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/requestsServer.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/requestsTypeObjectSupport.cxx
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_NAME}_interface
        pthread
        ${FAST_DDS_PACKAGE}
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
