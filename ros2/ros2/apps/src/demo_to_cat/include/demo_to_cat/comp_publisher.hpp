#ifndef DEMO_PUBLISHER_NODE_HPP_
#define DEMO_PUBLISHER_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "demo_interfaces/msg/vehicle.hpp"
#include <string>

namespace demo {

class PublisherNode : public rclcpp::Node
{
public:
  explicit PublisherNode(const rclcpp::NodeOptions options);
  ~PublisherNode();

  void SetDefaultParameters();

private:
  void on_timer();
  size_t count_;
  bool oper_;

  rclcpp::Publisher<demo_interfaces::msg::Vehicle>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  rclcpp::node_interfaces::PreSetParametersCallbackHandle::SharedPtr
    pre_set_parameters_callback_handle_;
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr
    on_set_parameters_callback_handle_;
  rclcpp::node_interfaces::PostSetParametersCallbackHandle::SharedPtr
    post_set_parameters_callback_handle_;

  double param_speed_scale_;
  double param_speed_step_;
};

}

#endif  // DEMO_PUBLISHER_NODE_HPP_
