#include "cat_apps/publisher_node.hpp"
#include "cat_msgs/msg/message.hpp"

using namespace std::chrono_literals;

namespace cat_apps {

PublisherNode::PublisherNode(
    const std::string &node_name,
    const std::string &topic_name,
    const uint32_t rate_ms,
    rclcpp::NodeOptions options)
: Node(node_name, options), m_currentValue(0), m_topic_name(topic_name), rate_ms(rate_ms)
{

}

PublisherNode::~PublisherNode() {
  if(m_timer) {
    m_timer->cancel();
  }
}

void PublisherNode::StartTimer()
{
  // Reliability: Reliable, Durability: Transient Local, History: Keep Last, Depth: 1
  rclcpp::QoS system_qos = rclcpp::QoS(1);
  system_qos.reliable().transient_local();

  m_publisher = this->create_publisher<cat_msgs::msg::Message>(this->m_topic_name, system_qos);
  m_timer = this->create_wall_timer(
    std::chrono::milliseconds(rate_ms),
    [this] {
            this->OnTimer();
    });
}


void PublisherNode::OnTimer()
{
    try {
        if (this->count_subscribers(this->m_topic_name) > 0) {
            Publish();
        }
    }
    catch (...)
    {
        std::cerr << "count subcribers failed\n";
    }

}

void PublisherNode::Publish()
{
  auto msg = cat_msgs::msg::Message();
  msg.additional_information = "Hello world!";

  m_publisher->publish(msg);
}

}
