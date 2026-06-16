// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ISignalConnection.h
// Description: Interface for boost::signal connections.

#ifndef MiddlewareInterfaceSignalConnection_H
#define MiddlewareInterfaceSignalConnection_H

namespace middleware
{
class ISignalConnection
{
public:
	virtual ~ISignalConnection() = default;
	virtual bool connected()     = 0;
	virtual void disconnect()    = 0;
};
}  // namespace middleware

#endif  // MiddlewareInterfaceSignalConnection_H
