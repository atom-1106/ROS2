// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareParticipant.h
// Description: Mock class for the MiddlewareParticipant interface.

#ifndef mockMiddlewareParticipant_H
#define mockMiddlewareParticipant_H

#include <gmock/gmock.h>
#include "IMiddlewareParticipant.h"

namespace mock
{
class MiddlewareParticipant : public ::middleware::IMiddlewareParticipant
{
public:
	virtual ~MiddlewareParticipant() override = default;
	MOCK_METHOD(std::unique_ptr<::middleware::IMiddlewareWriter>, CreateWriter, (std::string const&));
	MOCK_METHOD(
	    std::unique_ptr<::middleware::IMiddlewareReader>,
	    CreateReader,
	    (std::string const&,
	     ::middleware::IMiddlewareReader::SubscriptionCallback&&,
	     ::middleware::IMiddlewareReader::OnDisconnectCallback&&)
	);
};
}  // namespace mock

#endif  // mockMiddlewareParticipant_H
