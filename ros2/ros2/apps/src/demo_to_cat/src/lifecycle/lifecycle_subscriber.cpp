#include <functional>
#include <memory>
#include <string>

#include "lifecycle_msgs/msg/transition_event.hpp"

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"

class LifecycleSubscriberNode : public rclcpp::Node
{
public:
  explicit LifecycleSubscriberNode(const std::string & node_name)
  : Node(node_name)
  {
    // Data topic from the lc_talker node
    sub_data_ = this->create_subscription<std_msgs::msg::String>(
      "lifecycle_topic", 10,
      [this](std_msgs::msg::String::ConstSharedPtr msg) {
        return this->data_callback(msg);
      });

    sub_notification_ = this->create_subscription<lifecycle_msgs::msg::TransitionEvent>(
      "/lc_publisher_node/transition_event",
      10,
      [this](lifecycle_msgs::msg::TransitionEvent::ConstSharedPtr msg) {
        return this->notification_callback(msg);
      }
    );
  }

  void data_callback(std_msgs::msg::String::ConstSharedPtr msg)
  {
    RCLCPP_INFO(get_logger(), "data_callback: %s", msg->data.c_str());
  }

  void notification_callback(lifecycle_msgs::msg::TransitionEvent::ConstSharedPtr msg)
  {
    RCLCPP_INFO(
      get_logger(), "notify callback: Transition from state %s to %s",
      msg->start_state.label.c_str(), msg->goal_state.label.c_str());
  }

private:
  std::shared_ptr<rclcpp::Subscription<std_msgs::msg::String>> sub_data_;
  std::shared_ptr<rclcpp::Subscription<lifecycle_msgs::msg::TransitionEvent>> sub_notification_;
};

int main(int argc, char ** argv)
{
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  rclcpp::init(argc, argv);

  auto lc_listener = std::make_shared<LifecycleSubscriberNode>("lc_subscriber_node");
  rclcpp::spin(lc_listener);

  rclcpp::shutdown();

  return 0;
}
