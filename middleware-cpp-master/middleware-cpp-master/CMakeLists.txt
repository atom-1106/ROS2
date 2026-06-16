# Copyright (C) Caterpillar Inc. All Rights Reserved.

cmake_minimum_required(VERSION 3.20)

project(middleware)

# Project required variables
get_filename_component(MIDDLEWARE_ROOT ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
get_filename_component(MIDDLEWARE_PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/middleware-proto" ABSOLUTE)
get_filename_component(MIDDLEWARE_ARTIFACTS "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/${BUILD_TARGET}/${CMAKE_BUILD_TYPE}" ABSOLUTE)

# Preprocessor defines
add_compile_definitions(TGT_GCC_ARM64_GUEST=1)
add_compile_definitions(TGT_GCC_X86_WORKSTATION=2)
if("${BUILD_TARGET}" STREQUAL "tgt_gcc_arm64_guest")
    add_compile_definitions(BUILD_TARGET=1)
elseif("${BUILD_TARGET}" STREQUAL "tgt_gcc_x86_64_ubuntu_workstation")
    add_compile_definitions(BUILD_TARGET=2)
endif()

# Connects cmake files
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/build-utilities/${BUILD_TARGET}
)


# CMake includes
include(${BUILD_TARGET})

# Source code
add_subdirectory(idl)
add_subdirectory(lib)
add_subdirectory(apps)
