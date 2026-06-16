#ifndef CAT_APPS_SUBSCRIBER_NODE_H__
#define CAT_APPS_SUBSCRIBER_NODE_H__

#include "rclcpp/rclcpp.hpp"
#include "cat_msgs/msg/message.hpp"

#include <string>

namespace cat_apps {

class SubscriberNode : public rclcpp::Node
{
public:
  SubscriberNode(const std::string & node_name, const std::string & topic_name);
  ~SubscriberNode();

private:
  std::string const m_topic_name;
  rclcpp::Subscription<cat_msgs::msg::Message>::SharedPtr m_subscription;
};

}  // namespace cat_apps

#endif  // CAT_APPS_SUBSCRIBER_NODE_H__
