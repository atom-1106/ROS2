// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareReader.h
// Description: Mock class for the MiddlewareReader interface.

#ifndef mockMiddlewareReader_H
#define mockMiddlewareReader_H

#include <gmock/gmock.h>
#include "IMiddlewareReader.h"

namespace mock
{
class MiddlewareReader : public ::middleware::IMiddlewareReader
{
public:
	virtual ~MiddlewareReader() override = default;
	MOCK_METHOD(bool, ConnectedToPublisher, ());
	MOCK_METHOD(void, OnSubscriptionMatched, (bool));
	MOCK_METHOD(void, OnDataAvailable, ());
};
}  // namespace mock

#endif  // mockMiddlewareReader_H
