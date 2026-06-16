// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockBasicParameter.h
// Description: Mock classes for the Boost Signals wrappers.

#ifndef mockBoostSignals_H
#define mockBoostSignals_H

#include "ISignalConnection.h"
#include "ISignalling.h"

#include <gmock/gmock.h>

namespace mock
{
class SignalConnection : public ::middleware::ISignalConnection
{
public:
	virtual ~SignalConnection() override = default;
	MOCK_METHOD(bool, connected, ());
	MOCK_METHOD(void, disconnect, ());
};
}  // namespace mock

#endif  // mockBoostSignals_H
