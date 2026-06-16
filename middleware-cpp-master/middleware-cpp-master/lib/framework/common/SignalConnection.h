// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SignalConnection.h
// Description: Maintains the boost signal connection.

#ifndef MiddlewareSignalConnection_H
#define MiddlewareSignalConnection_H

#include <boost/signals2.hpp>
#include "ISignalConnection.h"

namespace middleware
{
class SignalConnection final : public ::middleware::ISignalConnection
{
public:
	explicit SignalConnection(boost::signals2::connection&& connection) : m_connection(connection)
	{
	}
	// this requires some explanation:
	//  when an object is destructing it is exactly that type, not a derived type.
	//  calling a virtual function from the destructor will dispatch to exactly that type
	//  and not the type it is derived from.
	//  By specifying the object-type and function to call this call should be
	//  resolved *at compile time* side-stepping the issue highlighted above.
	virtual ~SignalConnection() override
	{
		SignalConnection::disconnect();
	}
	bool connected() final override
	{
		return m_connection.connected();
	}
	void disconnect() final override
	{
		m_connection.disconnect();
	}

private:
	boost::signals2::connection m_connection;
};
}  // namespace middleware
#endif  // MiddlewareSignalConnection_H
