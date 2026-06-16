// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Main application for generating a few raw Fast DDS publishers

#include <pugixml.hpp>
#include "Subscriber.h"

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

std::mutex exit_mutex{};
bool exiting = false;
std::condition_variable condition{};

void signal_handler(int signal)
{
	if(signal == SIGUSR1 && std::string(std::getenv("EXIT_SIGUSR1")) == "true")
	{
		if(std::string(std::getenv("CLEAN_EXIT")) == "false")
		{
			exit_mutex.lock();
			std::quick_exit(0);
		}
		exiting = true;
		condition.notify_one();
	}
	if(signal == SIGUSR2)
	{
		exit_mutex.lock();
		std::quick_exit(0);
	}
}

int main(int argc, char** argv)
{
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	try
	{
		std::mutex mutex{};

		pugi::xml_document document;
		std::vector<std::unique_ptr<Subscriber>> subscribers{};

		if(document.load_file(argv[1]))
		{
			printf("Generating subscribers...\n");
			auto subNodes = document.select_node("/configuration/subscribers").node();
			for(auto const& parameter : subNodes.children())
			{
				std::string name = parameter.attribute("name").as_string();
				subscribers.emplace_back(std::make_unique<Subscriber>(name));
			}
		}
		else
		{
			throw std::invalid_argument("Could not parse the configuration\n");
		}
		printf("Subscribers generated were [%lu]\n", subscribers.size());

		{
			std::unique_lock<std::mutex> lock{mutex};
			condition.wait(lock, [] { return exiting; });
		}
		subscribers.clear();
	}
	catch(std::exception const& e)
	{
		std::cerr << e.what() << '\n';
	}
	return 0;
}
