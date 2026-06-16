#include "Publisher.hpp"
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
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

void run_no_threads(std::chrono::milliseconds const rate, std::vector<std::shared_ptr<Publisher>> const& publishers)
{
	while(!exiting)
	{
		for(auto& pub : publishers)
		{
			pub->publish();
		}
		std::this_thread::sleep_for(rate);
	}
}

void run_with_threads(std::chrono::milliseconds const rate, std::vector<std::shared_ptr<Publisher>> const& publishers)
{
	std::vector<std::thread> threads{};

	for(auto const& publisher : publishers)
	{
		threads.emplace_back(
		    [publisher, rate]()
		    {
			    while(!exiting)
			    {
				    publisher->publish();
				    std::this_thread::sleep_for(rate);
			    }
		    }
		);
	}

	std::mutex mut{};
	std::unique_lock<std::mutex> lock{mut};
	condition.wait(lock, [] { return exiting; });
	for(auto it = threads.begin(); it != threads.end(); ++it)
	{
		(*it).join();
	}
}

int main(int argc, char** argv)
{
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	std::vector<std::shared_ptr<Publisher>> publishers{};
	for(int i = 0; i < 3; i++)
	{
		publishers.emplace_back(std::make_unique<Publisher>(std::string{"Parameter"}.append(std::to_string(i))));
	}

	//run_no_threads(std::chrono::milliseconds{100}, publishers);
	run_with_threads(std::chrono::milliseconds{100}, publishers);
	publishers.clear();
	return 0;
}
