# COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
project(MiddlewareFrameworkMessageTopic)

add_library(${PROJECT_NAME}_interface INTERFACE)

target_link_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/lib
)

target_include_directories(${PROJECT_NAME}_interface INTERFACE
    ${ARTIFACTS_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Add Message Topic
add_library(${PROJECT_NAME} OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/messageCdrAux.ipp
    ${CMAKE_CURRENT_SOURCE_DIR}/messageTypeObjectSupport.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/messagePubSubTypes.cxx
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_NAME}_interface
        pthread
        ${FAST_DDS_PACKAGE}
        fastcdr
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
