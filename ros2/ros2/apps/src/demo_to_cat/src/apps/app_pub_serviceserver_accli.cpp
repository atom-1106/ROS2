
#include "demo_to_cat/pub_serviceserver_accli.hpp"
#include "demo_to_cat/constants.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::MultiThreadedExecutor exec(rclcpp::ExecutorOptions(), 2);

  rclcpp::NodeOptions options;

  auto node = std::make_shared<demo::PublisherServiceActionclientNode>(demo::APP_PUB_SER_ACCLI_NODE_NAME,
        demo::APP_PS_CS_ASC_TOPIC_NAME,
        demo::APP_PS_CS_ASC_SERVICE_NAME,
        demo::APP_PS_CS_ASC_ACTION_NAME,
        options);

  exec.add_node(node);
  exec.spin();

  rclcpp::shutdown();

  return 0;
}