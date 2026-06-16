cmake_minimum_required(VERSION 3.20)
project(demo_to_cat)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Custom Executable
function(custom_executable subfolder target)
  cmake_parse_arguments(ARG "" "" "DEPENDENCIES" ${ARGN})

  add_executable(${target} src/${subfolder}/${target}.cpp)

  target_include_directories(${target} PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
  )

  target_link_libraries(${target} PRIVATE
    ${ARG_DEPENDENCIES}
  )

  install(TARGETS ${target}
    DESTINATION lib/${PROJECT_NAME}
  )
endfunction()

# Custom RCL Components
function(create_comp_library plugin executable)
  cmake_parse_arguments(ARG "" "" "FILES;DEPENDENCIES" ${ARGN})

  set(library ${executable}_library)

  add_library(${library} SHARED ${ARG_FILES})

  target_include_directories(${library} PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
  )

  target_link_libraries(${library} PRIVATE
    ${ARG_DEPENDENCIES}
  )

  install(TARGETS ${library}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

  set(DEMO_EXECUTOR "rclcpp::executors::SingleThreadedExecutor" CACHE STRING "The executor for the demo nodes")

  message(STATUS "Setting executor of ${executable} to ${DEMO_EXECUTOR}")

  rclcpp_components_register_node(${library}
    PLUGIN ${plugin}
    EXECUTABLE ${executable}
    EXECUTOR ${DEMO_EXECUTOR}
  )
endfunction()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
# rclcpp_components::component
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(demo_interfaces REQUIRED)

#Lifecycle
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
###

include_directories(include)

add_library(pubsub
  src/pubsub/publisher.cpp
  src/pubsub/subscriber.cpp
)
target_link_libraries(pubsub PUBLIC rclcpp::rclcpp ${demo_interfaces_TARGETS})

add_library(services
  src/services/service_client.cpp
  src/services/service_server.cpp
)
target_link_libraries(services PUBLIC rclcpp::rclcpp ${demo_interfaces_TARGETS})

# add_library(dualsub
#   src/pubsub/dual_subscriber.cpp
# )
# target_link_libraries(dualsub PUBLIC rclcpp::rclcpp ${std_msgs_TARGETS})

add_library(action_server
  src/actions/action_server.cpp
)
target_link_libraries(action_server PUBLIC rclcpp::rclcpp rclcpp_action::rclcpp_action ${demo_interfaces_TARGETS})

add_library(action_client
  src/actions/action_client.cpp
)
target_link_libraries(action_client PUBLIC rclcpp::rclcpp rclcpp_action::rclcpp_action ${demo_interfaces_TARGETS})

add_library(sub_serviceclient
  src/combined/sub_serviceclient.cpp
)
target_link_libraries(sub_serviceclient PUBLIC rclcpp::rclcpp ${demo_interfaces_TARGETS})

add_library(pub_serviceserver_accli
  src/combined/pub_serviceserver_accli.cpp
)
target_link_libraries(pub_serviceserver_accli PUBLIC rclcpp::rclcpp rclcpp_action::rclcpp_action ${demo_interfaces_TARGETS})

add_library(sub_acser
  src/combined/sub_acser.cpp
)
target_link_libraries(sub_acser PUBLIC rclcpp::rclcpp rclcpp_action::rclcpp_action ${demo_interfaces_TARGETS})


install(TARGETS
  pubsub
  services
  # dualsub
  action_server
  action_client
  sub_serviceclient
  pub_serviceserver_accli
  sub_acser

  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

custom_executable(apps app_pubsub DEPENDENCIES rclcpp::rclcpp pubsub)
custom_executable(apps app_pub DEPENDENCIES rclcpp::rclcpp pubsub)
custom_executable(apps app_sub DEPENDENCIES rclcpp::rclcpp pubsub)
custom_executable(apps app_service_server DEPENDENCIES rclcpp::rclcpp services)
custom_executable(apps app_service_client DEPENDENCIES rclcpp::rclcpp services)
# custom_executable(apps dualsub_app DEPENDENCIES rclcpp::rclcpp dualsub pubsub)
custom_executable(apps app_action_server DEPENDENCIES rclcpp::rclcpp action_server)
custom_executable(apps app_action_client DEPENDENCIES rclcpp::rclcpp action_client)
custom_executable(apps app_sub_serviceclient DEPENDENCIES rclcpp::rclcpp sub_serviceclient)
custom_executable(apps app_pub_serviceserver_accli DEPENDENCIES rclcpp::rclcpp pub_serviceserver_accli)
custom_executable(apps app_sub_acser DEPENDENCIES rclcpp::rclcpp sub_acser)

# node information
custom_executable(nodes app_nodes DEPENDENCIES rclcpp::rclcpp)

# lifecycle
add_executable(lifecycle_publisher
  src/lifecycle/lifecycle_publisher.cpp
)

target_link_libraries(lifecycle_publisher PRIVATE
  rclcpp::rclcpp
  rclcpp_lifecycle::rclcpp_lifecycle
  ${lifecycle_msgs_TARGETS}
  ${std_msgs_TARGETS}
)

add_executable(lifecycle_subscriber
  src/lifecycle/lifecycle_subscriber.cpp
)

target_link_libraries(lifecycle_subscriber PRIVATE
  rclcpp::rclcpp
  ${lifecycle_msgs_TARGETS}
  ${std_msgs_TARGETS}
)

add_executable(lifecycle_service_client
  src/lifecycle/lifecycle_service_client.cpp
)

target_link_libraries(lifecycle_service_client PRIVATE
  ${lifecycle_msgs_TARGETS}
  rclcpp::rclcpp
)

install(TARGETS
  lifecycle_publisher
  lifecycle_subscriber
  lifecycle_service_client
  DESTINATION lib/${PROJECT_NAME}
)

create_comp_library("demo::PublisherNode" comp_pub
  FILES src/pubsub/comp_publisher.cpp
  DEPENDENCIES rclcpp::rclcpp rclcpp_components::component ${demo_interfaces_TARGETS}
)

create_comp_library("demo::SubscriberNode" comp_sub
  FILES src/pubsub/comp_subscriber.cpp
  DEPENDENCIES rclcpp::rclcpp rclcpp_components::component ${demo_interfaces_TARGETS}
)

# Install launch files.
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

# Install script files.
# install(DIRECTORY
#   script
#   DESTINATION share/${PROJECT_NAME}/
# )

ament_package()
