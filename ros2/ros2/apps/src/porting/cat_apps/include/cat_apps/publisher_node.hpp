#ifndef CAT_APPS_PUBLISHER_NODE_H__
#define CAT_APPS_PUBLISHER_NODE_H__

#include "rclcpp/rclcpp.hpp"
#include "cat_msgs/msg/message.hpp"

namespace cat_apps {

class PublisherNode : public rclcpp::Node
{
public:
  explicit PublisherNode(const std::string &node_name, const std::string &topic_name,
    const uint32_t rate_ms, rclcpp::NodeOptions options);
  ~PublisherNode();

  void StartTimer();

private:
  void OnTimer();
  void Publish();
  int32_t m_currentValue;
  const std::string m_topic_name;
  uint32_t rate_ms;

  rclcpp::Publisher<cat_msgs::msg::Message>::SharedPtr m_publisher;
  rclcpp::TimerBase::SharedPtr m_timer;
};

}

#endif /* CAT_APPS_PUBLISHER_NODE_H__ */