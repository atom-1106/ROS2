#include "cat_apps/subscriber_node.hpp"
#include "pugixml/pugixml.hpp"

#include <rclcpp/utilities.hpp>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char * argv[])
{
  const auto args = rclcpp::init_and_remove_ros_arguments(argc, argv);

  if (args.size() != 2) {
    std::fprintf(stderr, "Pass valid number of arguments\n");
    std::fprintf(stderr, "Usage:\n");
    std::fprintf(stderr, "./BaselineSubscriber <config.xml>\n");
    std::fprintf(stderr, "<config.xml> : configuration file\n");
    rclcpp::shutdown();
    return -1;
  }

  std::vector<std::shared_ptr<cat_apps::SubscriberNode>> subscribers;

  try {
    pugi::xml_document document;

    if (!document.load_file(args[1].c_str())) {
      throw std::invalid_argument("Could not parse the configuration\n");
    }

    std::printf("Generating subscribers...\n");
    auto sub_nodes = document.select_node("/configuration/subscribers").node();

    for (auto const & parameter : sub_nodes.children()) {
      const std::string name = parameter.attribute("name").as_string();
      const std::string node_name = "sub_" + name;
      subscribers.emplace_back(std::make_shared<cat_apps::SubscriberNode>(node_name, name));
    }

    std::printf("Subscribers generated were [%zu]\n", subscribers.size());
  } catch (std::exception const & e) {
    std::fprintf(stderr, "%s", e.what());
    rclcpp::shutdown();
    return 0;
  }

  if (subscribers.empty()) {
    std::fprintf(stderr, "NO subscribers are active, terminated the application.\n");
    rclcpp::shutdown();
    return 0;
  }

  rclcpp::executors::SingleThreadedExecutor executor;
  for (auto & subscriber : subscribers) {
    executor.add_node(subscriber);
  }

  executor.spin();
  rclcpp::shutdown();
  return 0;
}
