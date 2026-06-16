#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/service_server.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;

  rclcpp::NodeOptions options;

  auto server = std::make_shared<demo::ServerNode>(demo::APP_SERVER_NODE_NAME,
        demo::APP_SC_SERVICE_NAME,
        options);

  exec.add_node(server);
  exec.spin();

  rclcpp::shutdown();

  return 0;
}
