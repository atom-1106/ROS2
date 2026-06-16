
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/action_server.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;

  auto action_server = std::make_shared<demo::ActionServer>(demo::APP_ACTION_SERVER_NODE_NAME,
    demo::APP_SC_ACTION_NAME,
    options);

  rclcpp::spin(action_server);

  rclcpp::shutdown();
  return 0;
}