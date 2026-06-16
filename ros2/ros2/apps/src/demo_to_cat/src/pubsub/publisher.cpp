#include <chrono>

#include "demo_to_cat/publisher.hpp"

using namespace std::chrono_literals;

namespace demo {

PublisherNode::PublisherNode(
    const std::string &node_name,
    const std::string &topic_name,
    rclcpp::NodeOptions options)
: Node(node_name, options), count_(0)
{
  oper_ = true;
  // Reliability: Reliable, Durability: Volatile, History: Keep Last, Depth: 10
  rclcpp::QoS system_qos = rclcpp::SystemDefaultsQoS();
  publisher_ = create_publisher<demo_interfaces::msg::Vehicle>(topic_name, system_qos);
  timer_ = create_wall_timer(
    1000ms, std::bind(&PublisherNode::on_timer, this));

  SetDefaultParameters();
}

PublisherNode::~PublisherNode() {
  if(timer_) {
    timer_->cancel();
  }
}

void PublisherNode::SetDefaultParameters()
{
  this->declare_parameter("param_speed_scale", 0.1);
  param_speed_scale_ = this->get_parameter("param_speed_scale").as_double();
  param_speed_step_ = this->declare_parameter("param_speed_step", 0.0);

  auto modify_upcoming_parameters_callback =
    [](std::vector<rclcpp::Parameter> & parameters) {
      for (auto & param : parameters) {
        if (param.get_name() == "param_speed_scale") {
          parameters.push_back(rclcpp::Parameter("param_speed_step", 2.0));
        }
      }
    };

  auto validate_upcoming_parameters_callback =
    [](std::vector<rclcpp::Parameter> parameters) {
      rcl_interfaces::msg::SetParametersResult result;
      result.successful = true;

      for (const auto & param : parameters) {
        if (param.get_name() == "param_speed_scale") {
          if (param.get_value<double>() > 1.0) {
            result.successful = false;
            result.reason = "cannot set 'param_speed_scale' > 1.0";
            break;
          }
          if (param.get_value<double>() < 0.0) {
            result.successful = false;
            result.reason = "cannot set 'param_speed_scale' < 0.0";
            break;
          }
        } else if (param.get_name() == "param_speed_step") {
          if (param.get_value<double>() < 0.0) {
            result.successful = false;
            result.reason = "cannot set 'param_speed_step' < 0.0";
            break;
          }
          if (param.get_value<double>() > 10.0) {
            result.successful = false;
            result.reason = "cannot set 'param_speed_step' > 10.0";
            break;
          }
        }
      }

      return result;
    };

  auto react_to_updated_parameters_callback =
    [this](const std::vector<rclcpp::Parameter> & parameters) {
      for (const auto & param : parameters) {
        if (param.get_name() == "param_speed_scale") {
          param_speed_scale_ = param.get_value<double>();
          RCLCPP_INFO(get_logger(), "Member variable 'param_speed_scale_' set to: %f.", param_speed_scale_);
        }
        if (param.get_name() == "param_speed_step") {
          param_speed_step_ = param.get_value<double>();
          RCLCPP_INFO(get_logger(), "Member variable 'param_speed_step_' set to: %f.", param_speed_step_);
        }
      }
    };


  // Register the callbacks:
  pre_set_parameters_callback_handle_ = this->add_pre_set_parameters_callback(
    modify_upcoming_parameters_callback);
  on_set_parameters_callback_handle_ = this->add_on_set_parameters_callback(
    validate_upcoming_parameters_callback);
  post_set_parameters_callback_handle_ = this->add_post_set_parameters_callback(
    react_to_updated_parameters_callback);
}

void PublisherNode::on_timer()
{
  auto vehicle_msg = demo_interfaces::msg::Vehicle();
  vehicle_msg.vehicle.is_moving = true;
  float step = param_speed_step_;

  if(count_ <= 0) {
    count_ = 0;
    oper_ = true;
    vehicle_msg.vehicle.is_moving = false;
  }

  if(oper_) {
    count_+= 10;
  } else {
    count_-= 5;
  }

  if(count_ > 200) {
    oper_ = false;
  }
  if (param_speed_scale_ == 0.0) {
    vehicle_msg.vehicle.is_moving = false;
    step = 0.0;
  }
  vehicle_msg.vehicle.speed = static_cast<float>(count_) * param_speed_scale_ + step;
  RCLCPP_INFO(this->get_logger(), "[Pub]: Vehicle Moving: '%d', Speed: '%.2f'",
    vehicle_msg.vehicle.is_moving,
    vehicle_msg.vehicle.speed);
  publisher_->publish(vehicle_msg);
}

}
