#include <chrono>
#include <cinttypes>
#include <functional>
#include <future>
#include <memory>

#include "demo_to_cat/action_client.hpp"

namespace demo {

ActionClient::ActionClient(const std::string &node_name, const std::string &action_name,
  const rclcpp::NodeOptions &options)
: Node(node_name, options), goal_done_(false)
{
  this->declare_parameter("version", "10.0.2");

  this->client_ptr_ = rclcpp_action::create_client<UpdateFirmware>(
    this->get_node_base_interface(),
    this->get_node_graph_interface(),
    this->get_node_logging_interface(),
    this->get_node_waitables_interface(),
    action_name);

  this->timer_ = this->create_wall_timer(
    std::chrono::milliseconds(2000),
    std::bind(&ActionClient::SendGoal, this));
}

void ActionClient::SendGoal()
{
  using namespace std::placeholders;

  this->timer_->cancel();

  this->goal_done_ = false;

  if (!this->client_ptr_) {
    RCLCPP_ERROR(this->get_logger(), "Action client not initialized");
  }

  if (!this->client_ptr_->wait_for_action_server(std::chrono::seconds(10))) {
    RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
    this->goal_done_ = true;
    return;
  }

  // Goal
  auto goal_msg = UpdateFirmware::Goal();
  // version_ = this->get_parameter("version").as_string();
  goal_msg.firmware_version = this->get_parameter("version").as_string();

  RCLCPP_INFO(this->get_logger(), "Sending goal");

  auto send_goal_options = rclcpp_action::Client<UpdateFirmware>::SendGoalOptions();

  send_goal_options.goal_response_callback =
    std::bind(&ActionClient::GoalResponseCallback, this, _1);
  send_goal_options.feedback_callback =
    std::bind(&ActionClient::FeedbackCallback, this, _1, _2);
  send_goal_options.result_callback =
    std::bind(&ActionClient::ResultCallback, this, _1);

  auto goal_handle_future = this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
}

void ActionClient::GoalResponseCallback(GoalHandleUpdateFirmware::SharedPtr goal_handle)
{
  if (!goal_handle) {
    RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
  } else {
    RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
  }
}

void ActionClient::FeedbackCallback(
  GoalHandleUpdateFirmware::SharedPtr,
  const std::shared_ptr<const UpdateFirmware::Feedback> feedback)
{
  RCLCPP_INFO(this->get_logger(), "Feedback received: %s", feedback->current_phase.c_str());
}

void ActionClient::ResultCallback(const GoalHandleUpdateFirmware::WrappedResult &result)
{
  // this->goal_done_ = true;
  switch (result.code) {
    case rclcpp_action::ResultCode::SUCCEEDED:
      // Resume timer if succeeded
      this->timer_->reset();
      break;
    case rclcpp_action::ResultCode::ABORTED:
      RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
      return;
    case rclcpp_action::ResultCode::CANCELED:
      RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
      return;
    default:
      RCLCPP_ERROR(this->get_logger(), "Unknown result code");
      return;
  }

  RCLCPP_INFO(this->get_logger(), "Result received. Is update successful: %d", result.result->update_successful);
}

} // namespace demo
