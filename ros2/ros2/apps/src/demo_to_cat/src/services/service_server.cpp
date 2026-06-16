#include "demo_to_cat/service_server.hpp"
#include "demo_to_cat/constants.hpp"
#include <chrono>
#include <thread>

namespace demo {

ServerNode::ServerNode(const std::string &node_name, const std::string &service_name,
  const rclcpp::NodeOptions & options)
: Node(node_name, options)
{
  auto handle_sensor_calibration = [this](
    const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<SensorCalibration::Request> request,
    std::shared_ptr<SensorCalibration::Response> response) -> void
    {
      (void)request_header;
      RCLCPP_INFO(this->get_logger(), "Sensor Calibrate Request ID: %s", request->sensor_id.c_str());
      // DO some lookup table and calculate calibration based on given time
      std::this_thread::sleep_for(std::chrono::milliseconds(request->timeout));
      if(request->sensor_id == SENSOR_STEERING_ANGLE) {
          response->is_calibrated = true;
          response->error_margin = 0U;
          response->response = SENSOR_STEERING_ANGLE + " Successfully Calibrated";
      } else if(request->sensor_id == SENSOR_TEMPERATURE) {
          response->is_calibrated = true;
          response->response = SENSOR_TEMPERATURE + " Successfully Calibrated";
          response->error_margin = 0U;
      } else {
          response->is_calibrated = false;
          response->response = "Unknown Sensor Id to Calibration Failed";
          response->error_margin = 3U;
      }
    };
  // Create a service that will use the callback function to handle requests.
  srv_ = create_service<SensorCalibration>(service_name, handle_sensor_calibration);
}

}