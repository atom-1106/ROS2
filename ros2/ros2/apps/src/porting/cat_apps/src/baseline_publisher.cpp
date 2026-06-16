#include "cat_apps/publisher_coordinator.hpp"
#include "cat_apps/publisher_node.hpp"
#include "pugixml/pugixml.hpp"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char * argv[])
{
  if (argc != 3) {
    std::fprintf(stderr, "Pass valid number of arguments\n");
    std::fprintf(stderr, "Usage:\n");
    std::fprintf(stderr, "./BaselinePublisher <thread_mode> <config.xml>\n");
    std::fprintf(stderr, "<thread_mode> : 0 or 1 to run in single or multi threaded\n");
    std::fprintf(stderr, "<config.xml>  : configuration file\n");
    return -1;
  }

  rclcpp::init(argc, argv);

  std::vector<std::shared_ptr<cat_apps::PublisherNode>> publishers;
  bool run_threads = false;
  std::chrono::milliseconds rate{1000};

  try {
    run_threads = (std::string(argv[1]) == "1");
    pugi::xml_document document;

    if (!document.load_file(argv[2])) {
      throw std::invalid_argument("Could not parse the configuration\n");
    }

    std::printf("Generating publishers...\n");
    auto pub_nodes = document.select_node("/configuration/publishers").node();
    rate = std::chrono::milliseconds(pub_nodes.attribute("rate_ms").as_uint());

    for (auto const & parameter : pub_nodes.children()) {
      const std::string name = parameter.attribute("name").as_string();
      const uint32_t min_value = parameter.attribute("min").as_uint();
      const uint32_t max_value = parameter.attribute("max").as_uint();
      const std::string node_name = "pub_" + name;

      publishers.emplace_back(
        std::make_shared<cat_apps::PublisherNode>(node_name, name, min_value, max_value));
    }

    std::printf(
      "Publishers generated were [%zu] with a rate of [%ld] milliseconds.\n",
      publishers.size(),
      rate.count());
  } catch (std::exception const & e) {
    std::fprintf(stderr, "%s", e.what());
    rclcpp::shutdown();
    return 0;
  }

  if (publishers.empty()) {
    std::fprintf(stderr, "NO publishers are active, terminated the application.\n");
    rclcpp::shutdown();
    return 0;
  }

  std::unique_ptr<rclcpp::Executor> executor;
  rclcpp::Node::SharedPtr coordinator;

  if (run_threads) {
    std::printf("Running with separate threads.\n");
    for (auto & publisher : publishers) {
      publisher->StartTimer(static_cast<uint32_t>(rate.count()));
    }

    rclcpp::ExecutorOptions options;
    executor = std::make_unique<rclcpp::executors::MultiThreadedExecutor>(
      options, publishers.size());
  } else {
    std::printf("Running all in main thread.\n");
    coordinator = std::make_shared<cat_apps::PublisherCoordinator>(rate, publishers);
    executor = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
    executor->add_node(coordinator);
  }

  for (auto & publisher : publishers) {
    executor->add_node(publisher);
  }

  executor->spin();
  rclcpp::shutdown();
  return 0;
}
