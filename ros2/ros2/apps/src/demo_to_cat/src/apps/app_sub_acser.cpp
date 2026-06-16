#include "demo_to_cat/sub_acser.hpp"
#include "demo_to_cat/constants.hpp"

/* Having 1 Node: Includes: subscriber and action_server */

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::MultiThreadedExecutor exec(rclcpp::ExecutorOptions(), 2);

  rclcpp::NodeOptions options;

  auto node = std::make_shared<demo::SubscriberActionserverNode>(demo::APP_SUB_ACSER_NODE_NAME,
        demo::APP_PS_CS_ASC_TOPIC_NAME,
        demo::APP_PS_CS_ASC_ACTION_NAME,
        options);

  exec.add_node(node);
  exec.spin();

  rclcpp::shutdown();

  return 0;
}
