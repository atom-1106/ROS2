// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockPublishParameter.h
// Description: Mock class for the PublishParameter interface.

#ifndef mockPublishParameter_H
#define mockPublishParameter_H

#include <gmock/gmock.h>
#include "IPublishParameter.h"

namespace mock
{
class PublishParameter : public ::middleware::parameter::IPublish
{
public:
	virtual ~PublishParameter() override = default;
	MOCK_METHOD(void, Publish, (::middleware::parameter::Data const&));
	MOCK_METHOD(::middleware::parameter::Identity, Identity, ());
	MOCK_METHOD(::middleware::parameter::Unit, Unit, ());
};
}  // namespace mock

#endif  // mockPublishParameter_H
