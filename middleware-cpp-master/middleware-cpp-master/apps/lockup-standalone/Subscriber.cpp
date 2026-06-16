#include "Subscriber.hpp"
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

int main(int argc, char** argv)
{
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	std::vector<std::unique_ptr<Subscriber>> subscribers{};
	for(int i = 0; i < 3; i++)
	{
		subscribers.emplace_back(std::make_unique<Subscriber>(std::string{"Parameter"}.append(std::to_string(i))));
	}

	std::mutex mut{};
	std::unique_lock<std::mutex> lock{mut};
	condition.wait(lock, [] { return exiting; });
	subscribers.clear();
	return 0;
}
