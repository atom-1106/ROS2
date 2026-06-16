# Copyright (C) Caterpillar Inc. All Rights Reserved.
project(MiddlewareProtobuf)


set(PROTO_FILES
    "${MIDDLEWARE_PROTO_ROOT}/middleware_parameter.proto"
    "${MIDDLEWARE_PROTO_ROOT}/middleware_diagnostics.proto"
    "${MIDDLEWARE_PROTO_ROOT}/middleware_diagnostics_additionalinformation.proto"
)

add_library(${PROJECT_NAME} OBJECT)
# cannot pass this PROJECT_NAME as TARGET
# the header files would be matched by the target -> subdependencies using them would try to build before the copy
protobuf_generate(
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
    OUT_VAR PROTO_GENERATED_FILES
    APPEND_PATH
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf abseil::abseil)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Copy the results to the protobuf include directory, because source files can't be added as system files
file(MAKE_DIRECTORY ${PROTOBUF_INCLUDE_DIR})
foreach(f IN LISTS PROTO_GENERATED_FILES)
	if(f MATCHES "\\.h$")
		get_filename_component(f_basename ${f} NAME)
		add_custom_command(
		    OUTPUT "${PROTOBUF_INCLUDE_DIR}/${f_basename}"
		    COMMAND ${CMAKE_COMMAND} -E copy ${f} ${PROTOBUF_INCLUDE_DIR}
		    DEPENDS ${f})
		# the above has to be wrapped in a target - there was cmake issue 14633, but that tracker isn't used anymore
		# not sure if there is a better way to do it but I could not find any cleaner way to do all of this
		add_custom_target(Target_${f_basename} DEPENDS "${PROTOBUF_INCLUDE_DIR}/${f_basename}")
		add_dependencies(${PROJECT_NAME} Target_${f_basename})
	else()
		target_sources(${PROJECT_NAME} PRIVATE ${f})
	endif()
endforeach()
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE ${PROTOBUF_INCLUDE_DIR})
