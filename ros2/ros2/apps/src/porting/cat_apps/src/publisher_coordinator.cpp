#include "cat_apps/publisher_coordinator.hpp"

namespace cat_apps {

PublisherCoordinator::PublisherCoordinator(
  std::chrono::milliseconds rate,
  std::vector<std::shared_ptr<PublisherNode>> publishers)
: Node("baseline_pub_coordinator"),
  m_publishers(std::move(publishers))
{
  m_timer = create_wall_timer(
    rate,
    [this]() {
      for (auto & publisher : m_publishers) {
        publisher->Publish();
      }
    });
}

}  // namespace cat_apps
