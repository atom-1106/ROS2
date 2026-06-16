#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "lifecycle_msgs/msg/state.hpp"
#include "lifecycle_msgs/msg/transition.hpp"
#include "lifecycle_msgs/srv/change_state.hpp"
#include "lifecycle_msgs/srv/get_state.hpp"

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

namespace demo {

struct TaskStatus {
    std::mutex mtx;
    std::condition_variable cv;
    bool completed = false;
    bool interrupted = false;
};

// which node to handle
static constexpr char const * LIFECYCLE_NODE = "lc_publisher_node";

static constexpr char const * NODE_GET_STATE_TOPIC = "lc_publisher_node/get_state";
static constexpr char const * NODE_CHANGE_STATE_TOPIC = "lc_publisher_node/change_state";

template<typename FutureT, typename WaitTimeT>
std::future_status wait_for_result(FutureT & future, WaitTimeT time_to_wait)
{
  auto end = std::chrono::steady_clock::now() + time_to_wait;
  std::chrono::milliseconds wait_period(100);
  std::future_status status = std::future_status::timeout;
  do {
    auto now = std::chrono::steady_clock::now();
    auto time_left = end - now;
    if (time_left <= std::chrono::seconds(0)) {break;}
    status = future.wait_for((time_left < wait_period) ? time_left : wait_period);
  } while (rclcpp::ok() && status != std::future_status::ready);
  return status;
}

class LifecycleServiceClientNode : public rclcpp::Node
{
public:
  explicit LifecycleServiceClientNode(const std::string & node_name)
  : Node(node_name)
  {

  }

  void init()
  {
    client_get_state_ = this->create_client<lifecycle_msgs::srv::GetState>(NODE_GET_STATE_TOPIC);
    client_change_state_ = this->create_client<lifecycle_msgs::srv::ChangeState>(NODE_CHANGE_STATE_TOPIC);
  }

  unsigned int get_state(std::chrono::seconds time_out = 3s)
  {
    auto request = std::make_shared<lifecycle_msgs::srv::GetState::Request>();

    if (!client_get_state_->wait_for_service(time_out)) {
      RCLCPP_ERROR(
        get_logger(),
        "Service %s is not available.",
        client_get_state_->get_service_name());
      return lifecycle_msgs::msg::State::PRIMARY_STATE_UNKNOWN;
    }

    auto future_result = client_get_state_->async_send_request(request).future.share();

    auto future_status = wait_for_result(future_result, time_out);

    if (future_status != std::future_status::ready) {
      RCLCPP_ERROR(
        get_logger(), "Server time out while getting current state for node %s", LIFECYCLE_NODE);
      return lifecycle_msgs::msg::State::PRIMARY_STATE_UNKNOWN;
    }

    // We have an succesful answer. So let's print the current state.
    if (future_result.get()) {
      RCLCPP_INFO(
        get_logger(), "Node %s has current state %s.",
        LIFECYCLE_NODE, future_result.get()->current_state.label.c_str());
      return future_result.get()->current_state.id;
    } else {
      RCLCPP_ERROR(
        get_logger(), "Failed to get current state for node %s", LIFECYCLE_NODE);
      return lifecycle_msgs::msg::State::PRIMARY_STATE_UNKNOWN;
    }
  }

  void interrupted(TaskStatus &status)
  {
    {
      std::lock_guard<std::mutex> lock(status.mtx);
      status.interrupted = true;
    }
    status.cv.notify_all();
  }

  void completed(TaskStatus &status)
  {
    {
      std::lock_guard<std::mutex> lock(status.mtx);
      status.completed = true;
    }
    status.cv.notify_all();
  }

  /// Invokes a transition
  bool change_state(std::uint8_t transition, std::chrono::seconds time_out = 3s)
  {
    auto request = std::make_shared<lifecycle_msgs::srv::ChangeState::Request>();
    request->transition.id = transition;

    if (!client_change_state_->wait_for_service(time_out)) {
      RCLCPP_ERROR(
        get_logger(),
        "Service %s is not available.",
        client_change_state_->get_service_name());
      return false;
    }

    // We send the request with the transition we want to invoke.
    auto future_result = client_change_state_->async_send_request(request).future.share();

    // Let's wait until we have the answer from the node.
    // If the request times out, we return an unknown state.
    auto future_status = wait_for_result(future_result, time_out);

    if (future_status != std::future_status::ready) {
      RCLCPP_ERROR(
        get_logger(), "Server time out while getting current state for node %s", LIFECYCLE_NODE);
      return false;
    }

    // We have an answer, let's print our success.
    if (future_result.get()->success) {
      RCLCPP_INFO(
        get_logger(), "Transition %d successfully triggered.", static_cast<int>(transition));
      return true;
    } else {
      RCLCPP_WARN(
        get_logger(), "Failed to trigger transition %u", static_cast<unsigned int>(transition));
      return false;
    }
  }

private:
  std::shared_ptr<rclcpp::Client<lifecycle_msgs::srv::GetState>> client_get_state_;
  std::shared_ptr<rclcpp::Client<lifecycle_msgs::srv::ChangeState>> client_change_state_;
};

/**
 * This is a little independent
 * script which triggers the
 * default lifecycle of a node.
 * It starts with configure, activate,
 * deactivate, activate, deactivate,
 * cleanup and finally shutdown
 */
void callee_script(std::shared_ptr<LifecycleServiceClientNode> lc_client, TaskStatus &status)
{
  rclcpp::WallRate time_between_state_changes(0.1);  // 10s

  using Transition = lifecycle_msgs::msg::Transition;
  std::vector<std::uint8_t> transitions;
  transitions.reserve(7);
  transitions.emplace_back(Transition::TRANSITION_CONFIGURE);
  transitions.emplace_back(Transition::TRANSITION_ACTIVATE);
  transitions.emplace_back(Transition::TRANSITION_DEACTIVATE);
  transitions.emplace_back(Transition::TRANSITION_ACTIVATE);
  transitions.emplace_back(Transition::TRANSITION_DEACTIVATE);
  transitions.emplace_back(Transition::TRANSITION_CLEANUP);
  transitions.emplace_back(Transition::TRANSITION_UNCONFIGURED_SHUTDOWN);

  bool is_interrupted = false;

  for (auto it = transitions.begin(); it != transitions.end(); it++)
  {
    is_interrupted = true;
    if (!rclcpp::ok()) {
      break;
    }
    if (!lc_client->change_state(*it)) {
      RCLCPP_INFO(lc_client->get_logger(), "Failed during changing to state: %d", *it);
      break;
    }
    if (!lc_client->get_state()) {
      RCLCPP_INFO(lc_client->get_logger(), "On Unknown State");
      break;
    }
    is_interrupted = false;

    time_between_state_changes.sleep();
  }

  if(is_interrupted) {
    lc_client->interrupted(status);
  } else {
    lc_client->completed(status);
  }
}

void wake_executor(rclcpp::executors::SingleThreadedExecutor & exec, TaskStatus &status)
{
  std::unique_lock<std::mutex> lock(status.mtx);
  status.cv.wait(lock, [&status] {
            return status.completed || status.interrupted;
  });
  // Wake the executor when the script is done
  // https://github.com/ros2/rclcpp/issues/1916
  exec.cancel();
  return;
}

}

int main(int argc, char ** argv)
{
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  rclcpp::init(argc, argv);

  auto lc_client = std::make_shared<demo::LifecycleServiceClientNode>("lc_client");
  lc_client->init();

  rclcpp::executors::SingleThreadedExecutor exe;
  exe.add_node(lc_client);

  demo::TaskStatus status;

  std::shared_future<void> script = std::async(
    std::launch::async,
    demo::callee_script,
    lc_client,
    std::ref(status)
  );

  auto wake_exec = std::async(
    std::launch::async,
    demo::wake_executor,
    std::ref(exe),
    std::ref(status)
  );

  exe.spin_until_future_complete(script);

  rclcpp::shutdown();

  return 0;
}
