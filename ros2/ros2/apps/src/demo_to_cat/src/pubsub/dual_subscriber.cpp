#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "demo_to_cat/dual_subscriber.hpp"

namespace demo {

DualSubcriberThreadedNode::DualSubcriberThreadedNode(const std::string &node_name, const std::string &topic_name,
    rclcpp::NodeOptions options)
  : Node(node_name, options)
{
    callback_group_subscriber1_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    callback_group_subscriber2_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    auto sub1_opt = rclcpp::SubscriptionOptions();
    sub1_opt.callback_group = callback_group_subscriber1_;

    auto sub2_opt = rclcpp::SubscriptionOptions();
    sub2_opt.callback_group = callback_group_subscriber2_;

    subscription1_ = this->create_subscription<std_msgs::msg::String>(
        topic_name, rclcpp::QoS(10),
        std::bind(&DualSubcriberThreadedNode::Subscriber1Cb, this, std::placeholders::_1),
        sub1_opt);

    subscription2_ = this->create_subscription<std_msgs::msg::String>(
        topic_name, rclcpp::QoS(10),
        std::bind(&DualSubcriberThreadedNode::Subscriber2Cb, this, std::placeholders::_1),
        sub2_opt);
}

std::string DualSubcriberThreadedNode::TimingString()
{
    rclcpp::Time time = this->now();
    return std::to_string(time.nanoseconds());
}

std::string DualSubcriberThreadedNode::StringThreadId()
{
    static std::vector<std::thread::id> known_thread_ids {};
    const auto thread_it = std::find(known_thread_ids.cbegin(), known_thread_ids.cend(),
        std::this_thread::get_id());
    if (thread_it != known_thread_ids.cend()) {
        return std::to_string(std::distance(known_thread_ids.cbegin(), thread_it));
    }
    known_thread_ids.push_back(std::this_thread::get_id());
    return std::to_string(known_thread_ids.size() - 1);
}

void DualSubcriberThreadedNode::Subscriber1Cb(const std_msgs::msg::String::ConstSharedPtr msg)
{
    auto message_received_at = TimingString();

    // Extract current thread
    RCLCPP_INFO(this->get_logger(), "THREAD %s => Heard '%s' at %s",
        StringThreadId().c_str(), msg->data.c_str(), message_received_at.c_str());
}

void DualSubcriberThreadedNode::Subscriber2Cb(const std_msgs::msg::String::ConstSharedPtr msg)
{
    auto message_received_at = TimingString();

    // Prep display message
    RCLCPP_INFO(
        this->get_logger(), "THREAD %s => Heard '%s' at %s",
        StringThreadId().c_str(), msg->data.c_str(), message_received_at.c_str());
}

}