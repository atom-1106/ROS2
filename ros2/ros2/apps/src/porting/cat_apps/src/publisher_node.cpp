#include "cat_apps/publisher_node.hpp"
#include "cat_apps/message_codec.hpp"

#include <cstdio>
#include <utility>

namespace cat_apps {

namespace {

rclcpp::QoS BaselineQos()
{
  rclcpp::QoS qos(1);
  qos.reliable();
  qos.transient_local();
  return qos;
}

}  // namespace

PublisherNode::PublisherNode(
  const std::string & node_name,
  const std::string & topic_name,
  uint32_t min_value,
  uint32_t max_value)
: Node(node_name),
  m_topic_name(topic_name),
  m_min_value(min_value),
  m_max_value(max_value),
  m_current_value(min_value)
{
  rclcpp::PublisherOptions pub_options;
  pub_options.event_callbacks.matched_callback =
    [topic = m_topic_name](rclcpp::MatchedInfo & info) {
      if (info.current_count_change == 1) {
        std::printf("[%s] Connected to subscriber.\n", topic.c_str());
      } else if (info.current_count_change == -1) {
        std::printf("[%s] Disconnected from subscriber.\n", topic.c_str());
      } else {
        std::printf("[%s] Match error.\n", topic.c_str());
      }
    };

  m_publisher = create_publisher<cat_msgs::msg::Message>(m_topic_name, BaselineQos(), pub_options);
  std::printf("[%s] Publisher created.\n", m_topic_name.c_str());
}

PublisherNode::~PublisherNode()
{
  if (m_timer) {
    m_timer->cancel();
  }
}

void PublisherNode::Publish()
{
  cat_msgs::msg::Message data;
  data.additional_information = GetSystemTime();
  data.protobuf = ToUint32Bytes(m_current_value);
  m_publisher->publish(data);

  if (m_current_value == m_max_value) {
    m_current_value = m_min_value;
  } else {
    ++m_current_value;
  }
}

void PublisherNode::StartTimer(uint32_t rate_ms)
{
  m_timer = create_wall_timer(
    std::chrono::milliseconds(rate_ms),
    [this]() {
      OnTimer();
    });
}

void PublisherNode::OnTimer()
{
  Publish();
}

}  // namespace cat_apps
