# Copyright (C) Caterpillar Inc. All Rights Reserved.
project(MiddlewareLib)

set(MIDDLEWARE_LIB_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(framework)
add_subdirectory(parameters)
add_subdirectory(diagnostics)

if("${BUILD_TARGET}" STREQUAL "tgt_gcc_x86_64_ubuntu_workstation" AND NOT ${CONAN_PACKAGE})
    add_subdirectory(test)
endif()
