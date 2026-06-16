cmake_minimum_required(VERSION 3.8)
project(cat_apps)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cat_msgs REQUIRED)
find_package(pugixml REQUIRED)

add_executable(BaselinePublisher
  src/baseline_publisher.cpp
  src/publisher_node.cpp
  src/publisher_coordinator.cpp
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
)

install(FILES
  src/config-example.xml
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS
  BaselinePublisher
  BaselineSubscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
