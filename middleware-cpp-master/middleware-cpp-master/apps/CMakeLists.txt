# Copyright (C) Caterpillar Inc. All Rights Reserved.
cmake_minimum_required(VERSION 3.20)

project(MiddlewareTools)

include_directories(${MIDDLEWARE_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_subdirectory(DiscoveryServer)
# NOTE: Dependent on private ShmCleanup.h header
add_subdirectory(ShmCleanup)
# NOTE: Dependent on Spy Framework
add_subdirectory(Spy)

if(NOT ${CONAN_PACKAGE})
    add_subdirectory(Mapper)
    add_subdirectory(middleware-parameters-tester)
    add_subdirectory(baseline-publishers)
    add_subdirectory(baseline-subscribers)
    add_subdirectory(lockup-publisher)
    add_subdirectory(lockup-subscriber)
    add_subdirectory(stress-tester)
endif()
    
if("${BUILD_TARGET}" STREQUAL "tgt_gcc_x86_64_ubuntu_workstation" AND NOT ${CONAN_PACKAGE})
    add_subdirectory(integration-test)
endif()

# NOTE: Not sure what value lays heres, may also need removed
#add_subdirectory(ParameterPublish)
#add_subdirectory(ParameterSubscribe)
#add_subdirectory(ParameterReadClient)
#add_subdirectory(ParameterReadServer)

# NOTE: Broken before eProsima changes
#add_subdirectory(ParameterBuddyGen7CDAClient)
#add_subdirectory(ParameterBuddyGen7CDAServer)
