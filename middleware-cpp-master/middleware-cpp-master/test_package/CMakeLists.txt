cmake_minimum_required(VERSION 3.15)
message("Using CMAKE version: ${CMAKE_VERSION}")

project(MiddlewareTestPackage CXX)

find_package(cate_middleware REQUIRED)

add_executable(${PROJECT_NAME}_headers src/HeadersOnlyComponentMain.cpp)
# Note/Concern: Diagnostics header files have protobuf generated headers included, requires protobuf libs.
# 				This may need address later via a redesign.
target_link_libraries(${PROJECT_NAME}_headers cate_middleware::ServiceHeaders protobuf::libprotobuf abseil::abseil)

add_executable(${PROJECT_NAME}_parameters src/parameters/ParametersComponentMain.cpp)
target_link_libraries(${PROJECT_NAME}_parameters cate_middleware::ParameterService)
target_include_directories(${PROJECT_NAME}_parameters PRIVATE src/parameters)

add_executable(${PROJECT_NAME}_diagnostics src/diagnostics/DiagnosticsComponentMain.cpp)
target_link_libraries(${PROJECT_NAME}_diagnostics cate_middleware::DiagnosticsService)
target_include_directories(${PROJECT_NAME}_diagnostics PRIVATE src/diagnostics)

add_executable(${PROJECT_NAME}_primary src/PackageMain.cpp)
target_link_libraries(${PROJECT_NAME}_primary cate_middleware::cate_middleware)
target_include_directories(${PROJECT_NAME}_primary PRIVATE src/diagnostics src/parameters)

if("$ENV{BUILD_TARGET}" STREQUAL "tgt_gcc_x86_64_ubuntu_workstation")
	target_link_options(boost::boost INTERFACE "-Wl,--disable-new-dtags")
endif()
