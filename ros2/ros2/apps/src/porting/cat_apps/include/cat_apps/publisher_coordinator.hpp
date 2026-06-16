#ifndef CAT_APPS_PUBLISHER_COORDINATOR_H__
#define CAT_APPS_PUBLISHER_COORDINATOR_H__

#include "cat_apps/publisher_node.hpp"

#include <chrono>
#include <memory>
#include <vector>

namespace cat_apps {

class PublisherCoordinator : public rclcpp::Node
{
public:
  PublisherCoordinator(
    std::chrono::milliseconds rate,
    std::vector<std::shared_ptr<PublisherNode>> publishers);

private:
  std::vector<std::shared_ptr<PublisherNode>> m_publishers;
  rclcpp::TimerBase::SharedPtr m_timer;
};

}  // namespace cat_apps

#endif  // CAT_APPS_PUBLISHER_COORDINATOR_H__
