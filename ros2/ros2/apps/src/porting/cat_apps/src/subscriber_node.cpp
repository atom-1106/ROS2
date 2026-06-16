#include "cat_apps/subscriber_node.hpp"

namespace cat_apps {

SubscriberNode::SubscriberNode(const std::string &node_name, const std::string &topic_name,
    rclcpp::NodeOptions options)
: Node(node_name, options)
{

    m_timer = this->create_wall_timer(std::chrono::milliseconds(100),
        [this, topic_name]() {
                this->OnTimer(topic_name);
        });
}

SubscriberNode::~SubscriberNode()
{
    // clean up resources
}

void SubscriberNode::OnTimer(const std::string &topic_name)
{
    try
    {
        if(this->count_publishers(topic_name) > 0) {
            this->CheckPublisherAndCreateSubscriber(topic_name);
            this->m_timer->cancel();
        }
    }
    catch(...)
    {
        std::cerr << "Count Publishers error \n";
    }
}

void SubscriberNode::CheckPublisherAndCreateSubscriber(const std::string &topic_name)
{
    m_subscription = create_subscription<cat_msgs::msg::Message>(
        topic_name,
        10,
        [this](cat_msgs::msg::Message::UniquePtr msg) {
            RCLCPP_INFO(this->get_logger(), "[Received] = %s ", msg->additional_information.c_str());
        });
}

}
