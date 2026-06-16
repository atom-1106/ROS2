#include "demo_to_cat/service_client.hpp"
#include "demo_to_cat/constants.hpp"
#include <cinttypes>

namespace demo {

ClientNode::ClientNode(const std::string &node_name, const std::string &service_name,
  const rclcpp::NodeOptions & options)
: Node(node_name, options)
{
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  client_ = create_client<SensorCalibration>(service_name);
  QueueAsyncRequest(SENSOR_TEMPERATURE);
  timer_ = nullptr;
  OnTimerAsync();
}

void ClientNode::QueueAsyncRequest(const std::string &sensor_id)
{
  while (!client_->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
      return;
    }
    RCLCPP_INFO(this->get_logger(), "service not available, waiting again...");
  }
  auto request = std::make_shared<SensorCalibration::Request>();
  request->sensor_id = sensor_id;
  if(sensor_id == SENSOR_STEERING_ANGLE) {
    request->timeout = 30;
  } else if(sensor_id == SENSOR_TEMPERATURE) {
    request->timeout = 20;
  } else {
    request->timeout = 10;
  }

  using ServiceResponseFuture = rclcpp::Client<SensorCalibration>::SharedFuture;
  auto response_received_callback = [this](ServiceResponseFuture future) {
      auto result = future.get();
      RCLCPP_INFO(this->get_logger(), "ServiceClient:Is Calibrated = %d, Response = %s", result->is_calibrated, result->response.c_str());
    };
  auto future_result = client_->async_send_request(request, response_received_callback);
}

void ClientNode::SendRequest(const std::string &data)
{
  auto request = std::make_shared<SensorCalibration::Request>();

  request->sensor_id = data;

  auto result_future = client_->async_send_request(request);
  if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) != rclcpp::FutureReturnCode::SUCCESS)
  {
      RCLCPP_ERROR(this->get_logger(), "service call failed :(");
      client_->remove_pending_request(result_future);
  }
  else
  {
      auto result = result_future.get();
      RCLCPP_INFO(this->get_logger(), "ServiceClient:Is Calibrated = %d, Response = %s", result->is_calibrated, result->response.c_str());
  }
}

void ClientNode::OnTimer()
{
  static int counter = 0;
  if(timer_) {
    timer_->cancel();
    counter = 0;
  }

  auto timer_callback = [this]() -> void {
      ++counter;
      // bool enable_diable_flag = ((counter % 3) == 0);
      SendRequest("unknown_sensor_id");
    };
  timer_ = create_wall_timer(1000ms, timer_callback);
}

void ClientNode::OnTimerAsync()
{
  static int counter = 0;
  if(timer_) {
    timer_->cancel();
    counter = 0;
  }

  auto timer_callback = [this]() -> void {
      ++counter;
      if(counter % 3 == 0) {
        QueueAsyncRequest(SENSOR_STEERING_ANGLE);
      } else if(counter % 7 == 0) {
        QueueAsyncRequest("unknown_sensor_id");
      } else  {
        QueueAsyncRequest(SENSOR_TEMPERATURE);
      }
    };
  timer_ = create_wall_timer(1000ms, timer_callback);
}

ClientNode::~ClientNode()
{
  if(timer_) {
    timer_->cancel();
  }
}

}