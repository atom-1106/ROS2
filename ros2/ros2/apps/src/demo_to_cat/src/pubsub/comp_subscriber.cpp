#include "demo_to_cat/comp_subscriber.hpp"

namespace demo {

SubscriberNode::SubscriberNode(const rclcpp::NodeOptions options)
: Node("comp_sub_node", options)
{
  subscription_ = create_subscription<demo_interfaces::msg::Vehicle>(
    "comp_topic_name",
    10,
    [this](demo_interfaces::msg::Vehicle::UniquePtr msg) {
      RCLCPP_INFO(this->get_logger(), "[Subscriber]:Is Vehicle Moving: '%d', Speed: '%.2f'",
        msg->vehicle.is_moving,
        msg->vehicle.speed);
    });
}

}

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(demo::SubscriberNode)
