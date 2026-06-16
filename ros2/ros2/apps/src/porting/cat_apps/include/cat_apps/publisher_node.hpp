#ifndef CAT_APPS_PUBLISHER_NODE_H__
#define CAT_APPS_PUBLISHER_NODE_H__

#include "rclcpp/rclcpp.hpp"
#include "cat_msgs/msg/message.hpp"

#include <cstdint>
#include <string>

namespace cat_apps {

class PublisherNode : public rclcpp::Node
{
public:
  PublisherNode(
    const std::string & node_name,
    const std::string & topic_name,
    uint32_t min_value,
    uint32_t max_value);
  ~PublisherNode();

  void Publish();
  void StartTimer(uint32_t rate_ms);

  const std::string & topic_name() const {return m_topic_name;}

private:
  void OnTimer();

  std::string const m_topic_name;
  uint32_t const m_min_value;
  uint32_t const m_max_value;
  uint32_t m_current_value;

  rclcpp::Publisher<cat_msgs::msg::Message>::SharedPtr m_publisher;
  rclcpp::TimerBase::SharedPtr m_timer;
};

}  // namespace cat_apps

#endif  // CAT_APPS_PUBLISHER_NODE_H__
