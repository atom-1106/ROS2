#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/subscriber.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;

  rclcpp::NodeOptions options;

  auto sub1 = std::make_shared<demo::SubscriberNode>(demo::APP_SUB_NODE_NAME,
        demo::APP_PS_TOPIC_NAME,
        options);

  exec.add_node(sub1);
  exec.spin();

  rclcpp::shutdown();

  return 0;
}
