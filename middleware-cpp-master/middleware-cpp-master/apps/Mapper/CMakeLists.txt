# Copyright (C) Caterpillar Inc. All Rights Reserved.

project(Mapper)

add_executable(${PROJECT_NAME} 
	Mapper.cpp
	ParameterRepublisher.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	MiddlewareParameterService
    ${PUGIXML_PACKAGE}
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${APPLICATIONS_BINARY_DIR})
