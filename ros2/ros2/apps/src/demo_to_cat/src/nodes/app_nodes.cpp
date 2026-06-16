#include "rclcpp/rclcpp.hpp"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

using namespace std::chrono_literals;

namespace demo {

class NodeDetails : public rclcpp::Node
{
public:
  explicit NodeDetails(const std::string &node_name, const rclcpp::NodeOptions options)
  : Node(node_name, options)
  {
    timer_ = create_wall_timer(
    1000ms, std::bind(&NodeDetails::on_timer, this));
  }

  ~NodeDetails() {

  }

  void on_timer() {
    timer_->cancel();
    auto nodes = this->get_node_names();
    RCLCPP_INFO(get_logger(), "Existing nodes: ");
    for(auto &node_name : nodes) {
        RCLCPP_INFO(get_logger(), "\t %s", node_name.c_str());
    }
  }

private:
    rclcpp::TimerBase::SharedPtr timer_;

};

}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;

  rclcpp::NodeOptions options;

  auto pub = std::make_shared<demo::NodeDetails>("my_node", options);

  exec.add_node(pub);

  exec.spin();

  rclcpp::shutdown();

  return 0;
}
