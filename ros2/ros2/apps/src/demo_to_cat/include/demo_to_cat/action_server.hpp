#ifndef DEMO_ACTION_SERVER_NODE_HPP_
#define DEMO_ACTION_SERVER_NODE_HPP_

#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "demo_interfaces/action/update_firmware.hpp"

namespace demo {

class ActionServer : public rclcpp::Node
{
public:
  using UpdateFirmware = demo_interfaces::action::UpdateFirmware;
  using GoalHandleUpdateFirmware = rclcpp_action::ServerGoalHandle<UpdateFirmware>;

  explicit ActionServer(const std::string &node_name, const std::string &action_name,
    const rclcpp::NodeOptions &options);

private:
  rclcpp_action::Server<UpdateFirmware>::SharedPtr action_server_;
  std::string version_;

  rclcpp_action::GoalResponse HandleGoal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const UpdateFirmware::Goal> goal);

  rclcpp_action::CancelResponse HandleCancel(
    const std::shared_ptr<GoalHandleUpdateFirmware> goal_handle);

  void Execute(const std::shared_ptr<GoalHandleUpdateFirmware> goal_handle);

  void HandleAccepted(const std::shared_ptr<GoalHandleUpdateFirmware> goal_handle);
};

}

#endif // DEMO_ACTION_SERVER_NODE_HPP_
