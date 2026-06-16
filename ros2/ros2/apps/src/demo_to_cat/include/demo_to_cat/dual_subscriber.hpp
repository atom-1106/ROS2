#ifndef DEMO_DUAL_SUB_THREADED_NODE_HPP_
#define DEMO_DUAL_SUB_THREADED_NODE_HPP_

#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

namespace demo {

class DualSubcriberThreadedNode : public rclcpp::Node
{
public:
  explicit DualSubcriberThreadedNode(const std::string &node_name, const std::string &topic_name,
    rclcpp::NodeOptions options);

private:

  std::string TimingString();

  std::string StringThreadId();

  void Subscriber1Cb(const std_msgs::msg::String::ConstSharedPtr msg);

  void Subscriber2Cb(const std_msgs::msg::String::ConstSharedPtr msg);

  rclcpp::CallbackGroup::SharedPtr callback_group_subscriber1_;
  rclcpp::CallbackGroup::SharedPtr callback_group_subscriber2_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription1_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription2_;
};

}

#endif // DEMO_DUAL_SUB_THREADED_NODE_HPP_