#include "cat_apps/subscriber_node.hpp"
#include "cat_apps/message_codec.hpp"

#include <cstdio>

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

SubscriberNode::SubscriberNode(const std::string & node_name, const std::string & topic_name)
: Node(node_name),
  m_topic_name(topic_name)
{
  rclcpp::SubscriptionOptions sub_options;
  sub_options.event_callbacks.matched_callback =
    [topic = m_topic_name](rclcpp::MatchedInfo & info) {
      if (info.current_count_change == 1) {
        std::printf("[%s] Connected to publisher.\n", topic.c_str());
      } else if (info.current_count_change == -1) {
        std::printf("[%s] Disconnected from publisher.\n", topic.c_str());
      } else {
        std::printf("[%s] Match error.\n", topic.c_str());
      }
    };

  m_subscription = create_subscription<cat_msgs::msg::Message>(
    m_topic_name,
    BaselineQos(),
    [topic = m_topic_name](cat_msgs::msg::Message::UniquePtr message) {
      if (!message) {
        std::printf("[%s] Message data is invalid.\n", topic.c_str());
        return;
      }

      const uint32_t value = FromUint32Bytes(message->protobuf);
      std::printf(
        "[%s] received [%s]=[%u]\n",
        message->additional_information.c_str(),
        topic.c_str(),
        value);
    },
    sub_options);

  std::printf("[%s] Subscriber created.\n", m_topic_name.c_str());
}

SubscriberNode::~SubscriberNode() = default;

}  // namespace cat_apps
