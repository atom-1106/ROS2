#ifndef DEMO_ACTION_CLIENT_NODE_HPP_
#define DEMO_ACTION_CLIENT_NODE_HPP_

#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "demo_interfaces/action/update_firmware.hpp"

namespace demo {

class ActionClient : public rclcpp::Node
{
public:
  using UpdateFirmware = demo_interfaces::action::UpdateFirmware;
  using GoalHandleUpdateFirmware = rclcpp_action::ClientGoalHandle<UpdateFirmware>;

  explicit ActionClient(const std::string &node_name, const std::string &action_name,
    const rclcpp::NodeOptions &options);

  void SendGoal();

private:
  rclcpp_action::Client<UpdateFirmware>::SharedPtr client_ptr_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool goal_done_;
  std::string version_;

  void GoalResponseCallback(GoalHandleUpdateFirmware::SharedPtr goal_handle);

  void FeedbackCallback(GoalHandleUpdateFirmware::SharedPtr,
    const std::shared_ptr<const UpdateFirmware::Feedback> feedback);

  void ResultCallback(const GoalHandleUpdateFirmware::WrappedResult &result);

};

}

#endif // DEMO_ACTION_CLIENT_NODE_HPP_
