cmake_minimum_required(VERSION 3.8)
project(cat_apps)

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
find_package(rclcpp REQUIRED)
find_package(cat_msgs REQUIRED)
find_package(pugixml REQUIRED)
# find_package(rcl_yaml_param_parser REQUIRED)
# find_package(rcutils REQUIRED)

# Baseline Publisher
add_executable(BaselinePublisher
  src/baseline_publisher.cpp
  src/publisher_node.cpp
)

target_include_directories(BaselinePublisher PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
)

target_link_libraries(BaselinePublisher
  PUBLIC
  ${pugixml_TARGETS}
  rclcpp::rclcpp
  ${cat_msgs_TARGETS}
)

# Baseline Subscriber
add_executable(BaselineSubscriber
  src/baseline_subscriber.cpp
  src/subscriber_node.cpp
)

target_include_directories(BaselineSubscriber PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
)

target_link_libraries(BaselineSubscriber
  PUBLIC
  ${pugixml_TARGETS}
  rclcpp::rclcpp
  ${cat_msgs_TARGETS}
  rcl_yaml_param_parser::rcl_yaml_param_parser
  rcutils::rcutils
)

install(FILES
  src/config-example.xml
  src/load_parameters.yaml
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  BaselinePublisher
  BaselineSubscriber
  DESTINATION lib/${PROJECT_NAME}
)


ament_package()
