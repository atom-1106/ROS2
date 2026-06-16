#include <chrono>
#include <cinttypes>
#include <functional>
#include <future>
#include <memory>
#include <sstream>

#include "demo_to_cat/pub_serviceserver_accli.hpp"
#include "demo_to_cat/constants.hpp"

namespace demo {

PublisherServiceActionclientNode::PublisherServiceActionclientNode(const std::string &node_name,
    const std::string &topic_name,
    const std::string &service_name,
    const std::string &action_name,
    rclcpp::NodeOptions options)
 : Node(node_name, options)
{
    publisher_ = create_publisher<Vehicle>(topic_name, 10);
    pub_timer_ = create_wall_timer(
        2000ms, std::bind(&PublisherServiceActionclientNode::on_timer, this));

    auto handle_set_bool = [this](
    const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<SensorCalibration::Request> request,
    std::shared_ptr<SensorCalibration::Response> response) -> void
    {
      (void)request_header;
      RCLCPP_INFO(this->get_logger(), "[Service]: Sensor Calibrate Request ID: %s", request->sensor_id.c_str());
      // DO some lookup table and calculate calibration based on given time
      std::this_thread::sleep_for(std::chrono::milliseconds(request->timeout));
      if(request->sensor_id == SENSOR_STEERING_ANGLE) {
          response->is_calibrated = true;
          response->error_margin = 0U;
          response->response = "Successfully Calibrated";
      } if(request->sensor_id == SENSOR_TEMPERATURE) {
          response->is_calibrated = false;
          response->response = "Calibrated Failed";
          response->error_margin = 2U;
      } else {
          response->is_calibrated = false;
          response->response = "Unknown Sensor Id to Calibrate!";
          response->error_margin = 3U;
      }
    };
    // Create a service that will use the callback function to handle requests.
    srv_ = create_service<SensorCalibration>(service_name, handle_set_bool);


    this->declare_parameter("order", 10);

    action_client_ = rclcpp_action::create_client<Fibonacci>(
        this->get_node_base_interface(),
        this->get_node_graph_interface(),
        this->get_node_logging_interface(),
        this->get_node_waitables_interface(),
        action_name);

    action_client_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(1000),
        std::bind(&PublisherServiceActionclientNode::SendGoal, this));
}

PublisherServiceActionclientNode::~PublisherServiceActionclientNode()
{
  if(pub_timer_) {
    pub_timer_->cancel();
  }

  if(action_client_timer_) {
    action_client_timer_->cancel();
  }
}

void PublisherServiceActionclientNode::on_timer()
{
    auto vehicle_msg = demo_interfaces::msg::Vehicle();
    vehicle_msg.vehicle.is_moving = true;

    if(count_ <= 100) {
      if (count_ <= 0) {
        count_ = 0;
        oper_ = true;
      }
      vehicle_msg.vehicle.is_moving = false;
    }
    if(oper_) {
      count_+= 100;
    } else {
      count_-= 50;
    }
    if(count_ > 1000) {
      oper_ = false;
    }
    vehicle_msg.powertrain.engine_rpm = static_cast<float>(count_);
    RCLCPP_INFO(this->get_logger(), "[Pub]: Vehicle Moving: '%d', Engine_RPM: '%.2f'",
      vehicle_msg.vehicle.is_moving,
      vehicle_msg.powertrain.engine_rpm);
    publisher_->publish(vehicle_msg);
}

void PublisherServiceActionclientNode::SendGoal()
{
  using namespace std::placeholders;

  this->action_client_timer_->cancel();

  this->goal_done_ = false;

  if (!this->action_client_) {
    RCLCPP_ERROR(this->get_logger(), "Action client not initialized");
  }

  if (!this->action_client_->wait_for_action_server(std::chrono::seconds(10))) {
    RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
    this->goal_done_ = true;
    return;
  }

  // Goal
  auto goal_msg = Fibonacci::Goal();
  // goal_msg.order = 10;
  order_ = this->get_parameter("order").as_int();
  goal_msg.order = static_cast<int32_t>(order_);

  RCLCPP_INFO(this->get_logger(), "Sending goal");

  auto send_goal_options = rclcpp_action::Client<Fibonacci>::SendGoalOptions();

  send_goal_options.goal_response_callback =
    std::bind(&PublisherServiceActionclientNode::GoalResponseCallback, this, _1);
  send_goal_options.feedback_callback =
    std::bind(&PublisherServiceActionclientNode::FeedbackCallback, this, _1, _2);
  send_goal_options.result_callback =
    std::bind(&PublisherServiceActionclientNode::ResultCallback, this, _1);

  auto goal_handle_future = this->action_client_->async_send_goal(goal_msg, send_goal_options);
}

void PublisherServiceActionclientNode::GoalResponseCallback(GoalHandleFibonacci::SharedPtr goal_handle)
{
  if (!goal_handle) {
    RCLCPP_ERROR(this->get_logger(), "[ActionClient]: Goal was rejected by server");
  } else {
    RCLCPP_INFO(this->get_logger(), "[ActionClient]: Goal accepted by server, waiting for result");
  }
}

void PublisherServiceActionclientNode::FeedbackCallback(GoalHandleFibonacci::SharedPtr,
        const std::shared_ptr<const Fibonacci::Feedback> feedback)
{
    RCLCPP_INFO(this->get_logger(), "[ActionClient]: Next number in sequence received: %" PRId32, feedback->sequence.back());
}

void PublisherServiceActionclientNode::ResultCallback(const GoalHandleFibonacci::WrappedResult &result)
{
  // this->goal_done_ = true;
  switch (result.code) {
    case rclcpp_action::ResultCode::SUCCEEDED:
      // Resume timer if succeeded
      this->action_client_timer_->reset();
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

  std::stringstream ss;
  for (auto number : result.result->sequence) {
    ss << number << " ";
  }
  RCLCPP_INFO(this->get_logger(), "Result received: %s", ss.str().c_str());
}

}