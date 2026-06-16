#ifndef DEMO_PUB_SER_ACCLI_NODE_HPP_
#define DEMO_PUB_SER_ACCLI_NODE_HPP_

// Publisher, service server and action client
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "demo_interfaces/msg/vehicle.hpp"
#include "demo_interfaces/action/fibonacci.hpp"
#include "demo_interfaces/srv/sensor_calibration.hpp"

using namespace std::chrono_literals;

namespace demo {

using Vehicle = demo_interfaces::msg::Vehicle;
using Fibonacci = demo_interfaces::action::Fibonacci;
using GoalHandleFibonacci = rclcpp_action::ClientGoalHandle<Fibonacci>;
using SensorCalibration = demo_interfaces::srv::SensorCalibration;

/* publisher, Service Server and Action Client */
class PublisherServiceActionclientNode : public rclcpp::Node
{
public:
  explicit PublisherServiceActionclientNode(const std::string &node_name,
    const std::string &topic_name,
    const std::string &service_name,
    const std::string &action_name,
    rclcpp::NodeOptions options);

  ~PublisherServiceActionclientNode();

  void SendGoal();

private:
  void on_timer();
  size_t count_;
  rclcpp::Publisher<Vehicle>::SharedPtr publisher_;
  rclcpp::Service<SensorCalibration>::SharedPtr srv_;
  rclcpp_action::Client<Fibonacci>::SharedPtr action_client_;
  rclcpp::TimerBase::SharedPtr pub_timer_;
  rclcpp::TimerBase::SharedPtr action_client_timer_;
  bool goal_done_;
  int64_t order_;
  bool oper_;

  void GoalResponseCallback(GoalHandleFibonacci::SharedPtr goal_handle);

  void FeedbackCallback(GoalHandleFibonacci::SharedPtr,
    const std::shared_ptr<const Fibonacci::Feedback> feedback);

  void ResultCallback(const GoalHandleFibonacci::WrappedResult &result);
};

}

#endif  // DEMO_PUB_SER_ACCLI_NODE_HPP_
