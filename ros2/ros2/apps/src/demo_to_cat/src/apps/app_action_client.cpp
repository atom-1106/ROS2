#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/action_client.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::executors::SingleThreadedExecutor executor;

  rclcpp::NodeOptions options;

  auto action_client = std::make_shared<demo::ActionClient>(demo::APP_ACTION_CLIENT_NODE_NAME,
    demo::APP_SC_ACTION_NAME,
    options);

  executor.add_node(action_client);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
