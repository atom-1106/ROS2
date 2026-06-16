cmake_minimum_required(VERSION 3.8)
project(process_launcher)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

find_package(pugixml REQUIRED)

add_executable(launcher src/launcher.cpp)

target_link_libraries(launcher
  PUBLIC
  ${pugixml_TARGETS}
)

install(FILES
  src/config-example.xml
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  launcher
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()