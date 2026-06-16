#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/dual_subscriber.hpp"
#include "demo_to_cat/publisher.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  // Pool size to 3
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 3);

  rclcpp::NodeOptions options;
  options.use_intra_process_comms(true);

  auto pubnode = std::make_shared<demo::PublisherNode>(demo::APP_DUALSUB_PUB_NODE_NAME,
    demo::APP_DUALSUB_TOPIC_NAME,
    options);
  auto subnode = std::make_shared<demo::DualSubcriberThreadedNode>(demo::APP_DUALSUB_NODE_NAME,
    demo::APP_DUALSUB_TOPIC_NAME,
    options);

  executor.add_node(pubnode);
  executor.add_node(subnode);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
