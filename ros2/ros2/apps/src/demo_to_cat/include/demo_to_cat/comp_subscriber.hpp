#ifndef DEMO_SUBSCRIBER_NODE_HPP_
#define DEMO_SUBSCRIBER_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "demo_interfaces/msg/vehicle.hpp"

#include <string>

namespace demo {

class SubscriberNode : public rclcpp::Node
{
public:
    SubscriberNode(const rclcpp::NodeOptions options);

private:
    rclcpp::Subscription<demo_interfaces::msg::Vehicle>::SharedPtr subscription_;
};

}

#endif  // DEMO_SUBSCRIBER_NODE_HPP_
