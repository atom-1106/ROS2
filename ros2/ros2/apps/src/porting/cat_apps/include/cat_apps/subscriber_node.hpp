#ifndef CAT_APPS_SUBSCRIBER_NODE_H__
#define CAT_APPS_SUBSCRIBER_NODE_H__

#include "rclcpp/rclcpp.hpp"
#include "cat_msgs/msg/message.hpp"

namespace cat_apps {

class SubscriberNode : public rclcpp::Node
{
public:
    explicit SubscriberNode(const std::string &node_name, const std::string &topic_name,
        rclcpp::NodeOptions options);

    ~SubscriberNode();

private:

    void OnTimer(const std::string &topic_name);
    void CheckPublisherAndCreateSubscriber(const std::string &topic_name);
    rclcpp::Subscription<cat_msgs::msg::Message>::SharedPtr m_subscription;
    rclcpp::TimerBase::SharedPtr m_timer;
};

}

#endif /* CAT_APPS_SUBSCRIBER_NODE_H__ */