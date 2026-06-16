// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Finally.h
// Description: Runs an std::function upon destruction.

#ifndef Finally_H
#define Finally_H

#include <functional>
#include <memory>
#include <utility>

namespace testing
{

class Finally
{
	Finally(std::function<void(void)>&& onDestruct) : m_onDestruct{std::move(onDestruct)} {};

public:
	static std::shared_ptr<Finally> Do(std::function<void(void)>&& onDestruct)
	{
		return std::shared_ptr<Finally>{new Finally(std::move(onDestruct))};
	}
	~Finally()
	{
		m_onDestruct();
	}

private:
	std::function<void(void)> m_onDestruct;
};

}  // namespace testing

#endif  // Finally_H
