
#include "cat_apps/publisher_node.hpp"
#include "pugixml/pugixml.hpp"
#include <thread>
#include <string>
#include <chrono>
#include <vector>
#include <memory>
#include <iostream>

int main(int argc, char *argv[])
{
    static_cast<void>(argc);
    if (argc != 3) {
        std::cerr << "Pass valid number of arguments\n";
		std::cerr << "Usage: \n";
		std::cerr << "./<app_name> <arg1> <arg2>\n";
		std::cerr << "<arg1> : 0 or 1 to run in single or multi threaded \n";
		std::cerr << "<arg2> : configuration file, .xml file as input \n";
        return -1;
    }

	rclcpp::init(argc, argv);

	std::vector<rclcpp::Node::SharedPtr> publishers{};
	bool run_threads = false;

    try
	{
		std::string run_option(argv[1]);
		run_threads = (run_option == "1");
		pugi::xml_document document;
		uint32_t rate{1000};

		if(document.load_file(argv[2]))
		{
			printf("Generating publisher nodes ...\n");
			auto pubNodes = document.select_node("/configuration/publishers").node();
			for(auto const& node : pubNodes.children())
			{
				std::string node_name = node.attribute("name").as_string();
				std::string topic_name = node.attribute("topic").as_string();
				rate = node.attribute("rate_ms").as_uint();

				rclcpp::NodeOptions options;

				options.arguments({
						"--ros-args",
						"--params-file", "load_parameters.yaml"
					});

				options.automatically_declare_parameters_from_overrides(true);

				auto pub_node = std::make_shared<cat_apps::PublisherNode>(node_name, topic_name, rate, options);
				publishers.push_back(pub_node);
				pub_node->StartTimer();
			}
		}
		else
		{
			throw std::invalid_argument("Could not parse the configuration");
		}
		printf("Publishers generated were [%lu] with a rate of [%u] milliseconds.\n", publishers.size(), rate);
	}
	catch(std::exception const& e)
	{
		std::cerr << e.what() << '\n';
	}

	std::unique_ptr<rclcpp::Executor> executor;

	if(publishers.size() > 0)
	{
		if (run_threads)
		{
			rclcpp::ExecutorOptions options;
			size_t number_of_threads = 3;
			executor = std::make_unique<rclcpp::executors::MultiThreadedExecutor>(options, number_of_threads);
		} else {
			executor = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
		}
	}

	if(executor)
	{
		for(auto &node : publishers)
		{
			if(node)
			{
				executor->add_node(node);
			}
		}
		executor->spin();
		rclcpp::shutdown();
	}
	else
	{
		std::cerr << "NO publishers are active, terminated the application.\n";
	}

    return 0;
}