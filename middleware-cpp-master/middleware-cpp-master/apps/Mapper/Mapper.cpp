// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Mapper.cpp
// Description: Mapper : Republishes input parameters

#include <signal.h>
#include <atomic>
#include <chrono>
#include <initializer_list>
#include <thread>
#include "ParameterRepublisher.h"

namespace
{
using namespace std::chrono_literals;

constexpr auto n_loopDelay = 100ms;

std::atomic<bool> n_quit{false};
void Quit(int)
{
	n_quit = true;
}

void QuitInit()
{
	for(auto signal : {SIGTERM, SIGINT})
	{
		struct sigaction action;
		sigaction(signal, nullptr, &action);
		action.sa_handler = &Quit;
		sigaction(signal, &action, nullptr);
	}
}
}  // namespace

int main()
{
	QuitInit();

	ParameterRepublisher parameterRepublisher{};
	parameterRepublisher.Start();

	while(!n_quit)
	{
		std::this_thread::sleep_for(n_loopDelay);
	}

	return 0;
}
