#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/publisher.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;

  rclcpp::NodeOptions options;

  auto pub = std::make_shared<demo::PublisherNode>(demo::APP_PUB_NODE_NAME,
        demo::APP_PS_TOPIC_NAME,
        options);

  exec.add_node(pub);

  exec.spin();

  rclcpp::shutdown();

  return 0;
}
