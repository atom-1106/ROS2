
#include "demo_to_cat/sub_serviceclient.hpp"
#include "demo_to_cat/constants.hpp"

namespace demo {

SubscriberClientNode::SubscriberClientNode(const std::string &node_name,
        const std::string &topic_name,
        const std::string &service_name,
        rclcpp::NodeOptions options)
    : Node(node_name, options)
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    subscription_ = create_subscription<Vehicle>(
        topic_name,
        10,
        [this](Vehicle::UniquePtr msg) {
            RCLCPP_INFO(this->get_logger(), "[Subscriber]:Is Vehicle Moving: '%d', Engine RPM: '%.2f'",
              msg->vehicle.is_moving,
              msg->powertrain.engine_rpm);
        });

    client_ = create_client<SensorCalibration>(service_name);
    QueueAsyncRequest(SENSOR_TEMPERATURE);

    timer_ = nullptr;
    OnTimerAsync();

}

void SubscriberClientNode::OnTimer()
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

void SubscriberClientNode::OnTimerAsync()
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
        QueueAsyncRequest(SENSOR_TEMPERATURE);
      } else {
        QueueAsyncRequest("unknown_sensor_id");
      }
    };
  timer_ = create_wall_timer(1000ms, timer_callback);
}

SubscriberClientNode::~SubscriberClientNode()
{
  if(timer_) {
    timer_->cancel();
  }
}

void SubscriberClientNode::QueueAsyncRequest(const std::string &sensor_id)
{
    while (!client_->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(this->get_logger(), "[ServiceClient]:Interrupted while waiting for the service. Exiting.");
      return;
    }
    RCLCPP_INFO(this->get_logger(), "[ServiceClient]: Service not available, waiting again...");
  }
  auto request = std::make_shared<SensorCalibration::Request>();
  request->sensor_id = sensor_id;
  if(sensor_id == SENSOR_STEERING_ANGLE) {
    request->timeout = 500;
  } else if(sensor_id == SENSOR_TEMPERATURE) {
    request->timeout = 200;
  } else {
    request->timeout = 10;
  }

  using ServiceResponseFuture = rclcpp::Client<SensorCalibration>::SharedFuture;
  auto response_received_callback = [this](ServiceResponseFuture future) {
      auto result = future.get();
      RCLCPP_INFO(this->get_logger(), "[ServiceClient]:\nIs Calibrated = %d\nResponse = %s", result->is_calibrated, result->response.c_str());
    };
  auto future_result = client_->async_send_request(request, response_received_callback);
}

void SubscriberClientNode::SendRequest(const std::string &data)
{
  auto request = std::make_shared<SensorCalibration::Request>();

  request->sensor_id = data;

  auto result_future = client_->async_send_request(request);
  if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) != rclcpp::FutureReturnCode::SUCCESS)
  {
      RCLCPP_ERROR(this->get_logger(), "[ServiceClient]: Service call failed :(");
      client_->remove_pending_request(result_future);
  }
  else
  {
      auto result = result_future.get();
      RCLCPP_INFO(this->get_logger(), "[ServiceClient]:\nIs Calibrated = %d\nResponse = %s", result->is_calibrated, result->response.c_str());
  }
}

}
