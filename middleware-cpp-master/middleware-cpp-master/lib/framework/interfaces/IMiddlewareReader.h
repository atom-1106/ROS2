// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IMiddlewareReader.h
// Description: Interface for DDS Subscribers.

#ifndef IMiddlewareReader_H
#define IMiddlewareReader_H

#include <cstdint>
#include <functional>
#include <vector>

namespace middleware
{
class IMiddlewareReader
{
public:
	using SubscriptionCallback                       = std::function<void(std::vector<uint8_t> const&)>;
	using OnDisconnectCallback                       = std::function<void()>;
	virtual ~IMiddlewareReader()                     = default;
	virtual bool ConnectedToPublisher()              = 0;
	virtual void OnSubscriptionMatched(bool matched) = 0;
	virtual void OnDataAvailable()                   = 0;
};
}  // namespace middleware
#endif  // IMiddlewareReader_H
