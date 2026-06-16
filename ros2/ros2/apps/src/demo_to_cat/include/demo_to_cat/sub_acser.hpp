#ifndef DEMO_SUB_ACSER_NODE_HPP_
#define DEMO_SUB_ACSER_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "demo_interfaces/msg/vehicle.hpp"
#include "demo_interfaces/action/fibonacci.hpp"

#include <string>

namespace demo {
using Vehicle = demo_interfaces::msg::Vehicle;
using Fibonacci = demo_interfaces::action::Fibonacci;
using GoalHandleFibonacci = rclcpp_action::ServerGoalHandle<Fibonacci>;

class SubscriberActionserverNode : public rclcpp::Node
{
public:
    explicit SubscriberActionserverNode(const std::string &node_name,
        const std::string &topic_name,
        const std::string &action_name,
        rclcpp::NodeOptions options);

    ~SubscriberActionserverNode();

private:
    rclcpp::Subscription<Vehicle>::SharedPtr subscription_;
    rclcpp_action::Server<Fibonacci>::SharedPtr action_server_;

    rclcpp_action::GoalResponse HandleGoal(
            const rclcpp_action::GoalUUID & uuid,
            std::shared_ptr<const Fibonacci::Goal> goal);

    rclcpp_action::CancelResponse HandleCancel(
            const std::shared_ptr<GoalHandleFibonacci> goal_handle);

    void Execute(const std::shared_ptr<GoalHandleFibonacci> goal_handle);

    void HandleAccepted(const std::shared_ptr<GoalHandleFibonacci> goal_handle);
};

}

#endif  // DEMO_SUBSCRIBER_NODE_HPP_
