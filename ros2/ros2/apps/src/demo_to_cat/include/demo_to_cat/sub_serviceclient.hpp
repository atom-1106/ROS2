#ifndef DEMO_SUBSCRIBER_CLIENT_NODE_HPP_
#define DEMO_SUBSCRIBER_CLIENT_NODE_HPP_

#include <string>
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "demo_interfaces/msg/vehicle.hpp"
#include "demo_interfaces/srv/sensor_calibration.hpp"

using namespace std::chrono_literals;

namespace demo {

using SensorCalibration = demo_interfaces::srv::SensorCalibration;
using Vehicle = demo_interfaces::msg::Vehicle;

// subscriber and service client
class SubscriberClientNode : public rclcpp::Node
{
public:
    explicit SubscriberClientNode(const std::string &node_name,
            const std::string &topic_name,
            const std::string &service_name,
            rclcpp::NodeOptions options);

    ~SubscriberClientNode();

    void OnTimer();

    void OnTimerAsync();

private:

  void QueueAsyncRequest(const std::string &data);

  void SendRequest(const std::string &data);

  rclcpp::Subscription<Vehicle>::SharedPtr subscription_;
  rclcpp::Client<SensorCalibration>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}

#endif  // DEMO_SUBSCRIBER_NODE_HPP_
