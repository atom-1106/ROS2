#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "demo_to_cat/publisher.hpp"
#include "demo_to_cat/subscriber.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;

  rclcpp::NodeOptions options;
  options.use_intra_process_comms(true);

  auto pub = std::make_shared<demo::PublisherNode>(demo::APP_PUBSUB_PUB_NODE_NAME,
        demo::APP_PUBSUB_TOPIC_NAME,
        options);
  auto sub1 = std::make_shared<demo::SubscriberNode>(demo::APP_PUBSUB_SUB_NODE_NAME + "_01",
        demo::APP_PUBSUB_TOPIC_NAME,
        options);
  auto sub2 = std::make_shared<demo::SubscriberNode>(demo::APP_PUBSUB_SUB_NODE_NAME + "_02",
        demo::APP_PUBSUB_TOPIC_NAME,
        options);

  exec.add_node(pub);
  exec.add_node(sub1);
  exec.add_node(sub2);

  exec.spin();

  rclcpp::shutdown();

  return 0;
}
