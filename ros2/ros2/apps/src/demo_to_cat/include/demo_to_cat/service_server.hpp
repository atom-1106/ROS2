#ifndef DEMO_SERVICE_NODE_HPP_
#define DEMO_SERVICE_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "demo_interfaces/srv/sensor_calibration.hpp"
#include <string>

namespace demo {

using SensorCalibration = demo_interfaces::srv::SensorCalibration;

class ServerNode : public rclcpp::Node
{
public:
  explicit ServerNode(const std::string &node_name, const std::string &service_name,
    const rclcpp::NodeOptions & options);

private:
  rclcpp::Service<SensorCalibration>::SharedPtr srv_;
};

}
#endif