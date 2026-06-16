#include "demo_to_cat/subscriber.hpp"

namespace demo {

SubscriberNode::SubscriberNode(const std::string &node_name, const std::string &topic_name,
    rclcpp::NodeOptions options)
: Node(node_name, options)
{
  subscription_ = create_subscription<demo_interfaces::msg::Vehicle>(
    topic_name,
    10,
    [this](demo_interfaces::msg::Vehicle::UniquePtr msg) {
      RCLCPP_INFO(this->get_logger(), "[Subscriber]:Is Vehicle Moving: '%d', Speed: '%.2f'",
        msg->vehicle.is_moving,
        msg->vehicle.speed);
    });
}

}
