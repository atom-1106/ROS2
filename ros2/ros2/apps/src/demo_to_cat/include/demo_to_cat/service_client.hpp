#include <string>
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "demo_interfaces/srv/sensor_calibration.hpp"

using namespace std::chrono_literals;

namespace demo
{

using SensorCalibration = demo_interfaces::srv::SensorCalibration;

class ClientNode : public rclcpp::Node
{
public:
  explicit ClientNode(const std::string &node_name,
                      const std::string &service_name,
                      const rclcpp::NodeOptions & options);

   ~ClientNode();

   void OnTimer();

   void OnTimerAsync();

private:

  void QueueAsyncRequest(const std::string &sensor_id);

  void SendRequest(const std::string &sensor_id);

  rclcpp::Client<SensorCalibration>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}
