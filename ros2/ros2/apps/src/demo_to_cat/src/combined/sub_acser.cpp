#include <functional>
#include <memory>
#include <thread>

#include "demo_to_cat/sub_acser.hpp"

namespace demo {

SubscriberActionserverNode::SubscriberActionserverNode(const std::string &node_name,
    const std::string &topic_name,
    const std::string &action_name,
    rclcpp::NodeOptions options)
 : Node(node_name, options)
{
  subscription_ = create_subscription<Vehicle>(
    topic_name,
    10,
    [this](Vehicle::UniquePtr msg) {
      RCLCPP_INFO(this->get_logger(), "[Subscriber]:Is Vehicle Moving: '%d', Engine RPM: '%.2f'",
        msg->vehicle.is_moving,
        msg->powertrain.engine_rpm);
    });

    using namespace std::placeholders;

  this->action_server_ = rclcpp_action::create_server<Fibonacci>(
    this->get_node_base_interface(),
    this->get_node_clock_interface(),
    this->get_node_logging_interface(),
    this->get_node_waitables_interface(),
    action_name,
    std::bind(&SubscriberActionserverNode::HandleGoal, this, _1, _2),
    std::bind(&SubscriberActionserverNode::HandleCancel, this, _1),
    std::bind(&SubscriberActionserverNode::HandleAccepted, this, _1));
}

SubscriberActionserverNode::~SubscriberActionserverNode()
{

}

rclcpp_action::GoalResponse SubscriberActionserverNode::HandleGoal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const Fibonacci::Goal> goal)
{
  RCLCPP_INFO(this->get_logger(), "Received goal request with order %d", goal->order);
  (void)uuid;
  // Let's reject sequences that are over 9000
  if (goal->order > 9000) {
    return rclcpp_action::GoalResponse::REJECT;
  }
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse SubscriberActionserverNode::HandleCancel(
        const std::shared_ptr<GoalHandleFibonacci> goal_handle)
{
  RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
  (void)goal_handle;
  return rclcpp_action::CancelResponse::ACCEPT;
}

void SubscriberActionserverNode::Execute(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
{
  RCLCPP_INFO(this->get_logger(), "Executing goal");
  rclcpp::Rate loop_rate(1);
  const auto goal = goal_handle->get_goal();
  auto feedback = std::make_shared<Fibonacci::Feedback>();
  auto & sequence = feedback->sequence;
  sequence.push_back(0);
  sequence.push_back(1);
  auto result = std::make_shared<Fibonacci::Result>();

  for (int i = 1; (i < goal->order) && rclcpp::ok(); ++i) {
    // Check if there is a cancel request
    if (goal_handle->is_canceling()) {
      result->sequence = sequence;
      goal_handle->canceled(result);
      RCLCPP_INFO(this->get_logger(), "Goal Canceled");
      return;
    }
    // Update sequence
    sequence.push_back(sequence[i] + sequence[i - 1]);
    // Publish feedback
    goal_handle->publish_feedback(feedback);
    RCLCPP_INFO(this->get_logger(), "Publish Feedback");

    loop_rate.sleep();
  }

  // Check if goal is done
  if (rclcpp::ok()) {
    result->sequence = sequence;
    goal_handle->succeed(result);
    RCLCPP_INFO(this->get_logger(), "Goal Succeeded");
  }
}

void SubscriberActionserverNode::HandleAccepted(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
{
  using namespace std::placeholders;
  // this needs to return quickly to avoid blocking the executor, so spin up a new thread
  std::thread{std::bind(&SubscriberActionserverNode::Execute, this, _1), goal_handle}.detach();
}

}