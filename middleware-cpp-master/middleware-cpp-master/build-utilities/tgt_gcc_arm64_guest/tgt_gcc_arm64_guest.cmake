# Copyright (C) Caterpillar Inc. All Rights Reserved.

#Compiler
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fdiagnostics-color=always) #Error and Warnings are colored

# Warnings
add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-fPIC)

#add_compile_options(-fconcepts)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

# Definitions
add_compile_definitions(BOOST_IOSTREAMS_USE_DEPRECATED) #boost file_descriptor constructor with file desciptor as argument
set(STRICT_REALTIME ON CACHE BOOL "" FORCE)

# Common paths
get_filename_component(APPLICATIONS_BINARY_DIR "${MIDDLEWARE_ARTIFACTS}/apps" ABSOLUTE)
get_filename_component(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${MIDDLEWARE_ARTIFACTS}/lib" ABSOLUTE)
get_filename_component(MIDDLEWARE_INCLUDE_DIR "${MIDDLEWARE_ARTIFACTS}/include" ABSOLUTE)
get_filename_component(PROTOBUF_INCLUDE_DIR "${MIDDLEWARE_ARTIFACTS}/include/protobuf" ABSOLUTE)


# conan
find_package(protobuf)
find_package(Boost)
find_package(pugixml)
find_package(fastdds)

# Required Global variables
#set(GRPC_LIBRARIES ${EXTERNAL_ROOT}/grpc-a/tgt_gcc_a7hx_guest/lib)
#set(GRPC_INCLUDES ${EXTERNAL_ROOT}/grpc-a/tgt_gcc_a7hx_guest/include)

# Required Global Packages
set(BOOST_PACKAGES boost::boost Boost::system)
set(PUGIXML_PACKAGE pugixml::pugixml)
set(FAST_DDS_PACKAGE fastdds)


