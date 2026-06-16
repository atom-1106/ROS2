// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockParameterService.h
// Description: Mock class for the ParameterService interface.

#ifndef mockParameterService_H
#define mockParameterService_H

#include <gmock/gmock.h>
#include <middleware_parameter.pb.h>
#include "IParameterService.h"

namespace mock
{
class ParameterService : public ::middleware::parameter::IService
{
public:
	virtual ~ParameterService() override = default;

	MOCK_METHOD(
	    ::middleware::parameter::IPublish&,
	    RetrievePublisherInstance,
	    (::middleware::parameter::Identity const&)
	);
	MOCK_METHOD(::middleware::parameter::IRead&, RetrieveReadInstance, (::middleware::parameter::Identity const&));
	MOCK_METHOD(::middleware::parameter::IWrite&, RetrieveWriteInstance, (::middleware::parameter::Identity const&));
	MOCK_METHOD(void, Reply, (uint_least16_t, ::middleware::parameter::Data const&));
};
}  // namespace mock

#endif  // mockParameterService_H
