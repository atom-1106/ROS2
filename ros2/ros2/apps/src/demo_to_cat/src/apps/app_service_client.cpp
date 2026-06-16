#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/service_client.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;

  rclcpp::NodeOptions options;

  auto client = std::make_shared<demo::ClientNode>(demo::APP_CLIENT_NODE_NAME,
        demo::APP_SC_SERVICE_NAME,
        options);

  exec.add_node(client);
  exec.spin();

  rclcpp::shutdown();

  return 0;
}
