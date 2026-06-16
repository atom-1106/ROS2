// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IParameterService.h
// Description: Interface API for external users.

#ifndef IParameterService_H
#define IParameterService_H

#include <cstdint>
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
class IPublish;
class IRead;
class IWrite;

class IService
{
public:
	virtual ~IService()                                                   = default;
	virtual IPublish& RetrievePublisherInstance(Identity const& identity) = 0;
	virtual IRead& RetrieveReadInstance(Identity const& identity)         = 0;
	virtual IWrite& RetrieveWriteInstance(Identity const& identity)       = 0;
	virtual void Reply(uint_least16_t key, Data const& data)              = 0;
};
}  // namespace parameter
}  // namespace middleware
#endif  // IParameterService_H
