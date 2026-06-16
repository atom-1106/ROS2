#include <functional>
#include <memory>
#include <thread>

#include "demo_to_cat/action_server.hpp"

namespace demo {

ActionServer::ActionServer(const std::string &node_name, const std::string &action_name,
  const rclcpp::NodeOptions &options)
: Node(node_name, options)
{
  using namespace std::placeholders;
  version_ = "10.0.0";

  this->action_server_ = rclcpp_action::create_server<UpdateFirmware>(
    this->get_node_base_interface(),
    this->get_node_clock_interface(),
    this->get_node_logging_interface(),
    this->get_node_waitables_interface(),
    action_name,
    std::bind(&ActionServer::HandleGoal, this, _1, _2),
    std::bind(&ActionServer::HandleCancel, this, _1),
    std::bind(&ActionServer::HandleAccepted, this, _1));
}


rclcpp_action::GoalResponse ActionServer::HandleGoal(
  const rclcpp_action::GoalUUID & uuid,
  std::shared_ptr<const UpdateFirmware::Goal> goal)
{
  RCLCPP_INFO(this->get_logger(),
    "Received goal request for firmware upgrade: version: %s, hw_module: %s, reboot: %d",
      goal->firmware_version.c_str(), goal->hardware_module.c_str(), goal->reboot);
  (void)uuid;
  // Check current firmware version and update: do some buisness logice over here
  if (goal->firmware_version == version_) {
    return rclcpp_action::GoalResponse::REJECT;
  }
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse ActionServer::HandleCancel(
  const std::shared_ptr<GoalHandleUpdateFirmware> goal_handle)
{
  RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
  (void)goal_handle;
  return rclcpp_action::CancelResponse::ACCEPT;
}

void ActionServer::Execute(const std::shared_ptr<GoalHandleUpdateFirmware> goal_handle)
{
  RCLCPP_INFO(this->get_logger(), "Executing goal");
  rclcpp::Rate loop_rate(1);
  const auto goal = goal_handle->get_goal();
  auto feedback = std::make_shared<UpdateFirmware::Feedback>();
  auto &current_phase = feedback->current_phase;
  auto result = std::make_shared<UpdateFirmware::Result>();
  current_phase = "Downloading...";

  for (int i = 1; (i < 11) && rclcpp::ok(); ++i) {
    // Check if there is a cancel request
    if (goal_handle->is_canceling()) {
      result->update_successful = false;
      result->error_code = 2;
      goal_handle->canceled(result);
      RCLCPP_INFO(this->get_logger(), "Goal Canceled");
      return;
    }
    if(i < 3) {
      current_phase = "Downloading...";
    } else if (i == 3) {
      current_phase = "Downloaded";
    } else if(i <= 8) {
      current_phase = "Installing...";
    } else if(i < 9) {
      current_phase = "Installed";
    } else {
      current_phase = "Fireware Upgraded.";
    }
    // Publish feedback
    goal_handle->publish_feedback(feedback);
    RCLCPP_INFO(this->get_logger(), "Publish Feedback");

    loop_rate.sleep();
  }

  // Check if goal is done
  if (rclcpp::ok()) {
    version_ = goal->firmware_version;
    result->update_successful = true;
    result->error_code = 0;
    goal_handle->succeed(result);
    RCLCPP_INFO(this->get_logger(), "Goal Succeeded");
  }
}

void ActionServer::HandleAccepted(const std::shared_ptr<GoalHandleUpdateFirmware> goal_handle)
{
  using namespace std::placeholders;
  // this needs to return quickly to avoid blocking the executor, so spin up a new thread
  std::thread{std::bind(&ActionServer::Execute, this, _1), goal_handle}.detach();
}

} // namespace demo
