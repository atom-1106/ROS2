// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Main application for generating a few raw Fast DDS publishers

#include <pugixml.hpp>
#include "Publisher.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

void RunNoThreads(std::chrono::milliseconds const rate, std::vector<std::shared_ptr<Publisher>> const& publishers)
{
	printf("Running all in main thread.\n");
	while(true)
	{
		for(auto& pub : publishers)
		{
			pub->Publish();
		}
		std::this_thread::sleep_for(rate);
	}
}
void RunWithThreads(std::chrono::milliseconds const rate, std::vector<std::shared_ptr<Publisher>> const& publishers)
{
	printf("Running with separate threads.\n");
	std::mutex mutex{};
	std::condition_variable condition{};
	std::vector<std::thread> threads{};

	for(auto const& pub : publishers)
	{
		threads.emplace_back(
		    [p = pub, r = rate]
		    {
			    while(true)
			    {
				    p->Publish();
				    std::this_thread::sleep_for(r);
			    }
		    }
		);
	}

	{
		std::unique_lock<std::mutex> lock{mutex};
		condition.wait(lock, [] { return false; });
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string run_option(argv[1]);
		bool run_threads = (run_option == "1" ? true : false);
		pugi::xml_document document;
		std::chrono::milliseconds rate{1000};
		std::vector<std::shared_ptr<Publisher>> publishers{};

		if(document.load_file(argv[2]))
		{
			printf("Generating publishers...\n");
			auto pubNodes = document.select_node("/configuration/publishers").node();
			rate          = std::chrono::milliseconds(pubNodes.attribute("rate_ms").as_uint());
			for(auto const& parameter : pubNodes.children())
			{
				std::string name = parameter.attribute("name").as_string();
				uint32_t min     = parameter.attribute("min").as_uint();
				uint32_t max     = parameter.attribute("max").as_uint();
				publishers.emplace_back(std::make_unique<Publisher>(name, min, max));
			}
		}
		else
		{
			throw std::invalid_argument("Could not parse the configuration\n");
		}
		printf("Publishers generated were [%lu] with a rate of [%ld] milliseconds.\n", publishers.size(), rate.count());

		if(run_threads)
		{
			RunWithThreads(rate, publishers);
		}
		else
		{
			RunNoThreads(rate, publishers);
		}
	}
	catch(std::exception const& e)
	{
		std::cerr << e.what() << '\n';
	}
	return 0;
}
