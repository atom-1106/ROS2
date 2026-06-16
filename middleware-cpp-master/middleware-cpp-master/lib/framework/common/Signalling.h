// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Signalling.h
// Description: The registered signal handler.

#ifndef MiddlewareSignalling_H
#define MiddlewareSignalling_H

#include "ISignalling.h"
#include "SignalConnection.h"

namespace middleware
{
template <class R, class... Args>
class Signalling;

template <class R, class... Args>
class Signalling<R(Args...)> : public ISignalling<R(Args...)>
{
public:
	using Callback        = typename ISignalling<R(Args...)>::Callback;
	using Registration    = typename ISignalling<R(Args...)>::Registration;
	using GroupType       = typename ISignalling<Signalling::Callback>::GroupType;
	using ConnectPosition = typename ISignalling<Signalling::Callback>::ConnectPosition;

	/// Connect to an event by ID.

	Registration Register(
	    std::function<Callback> const& callback,
	    GroupType const group                     = ISignalling<Signalling::Callback>::default_group,
	    ConnectPosition const connection_position = ISignalling<Signalling::Callback>::default_connect_position
	) final override
	{
		return std::make_unique<SignalConnection>(m_signal.connect(group, callback, connection_position));
	}

	/// Disconnect from an event.
	void Deregister(Registration& registration) final override
	{
		registration->disconnect();
	}

	R FireEvent(Args... args)
	{
		return m_signal(std::forward<Args>(args)...);
	}

private:
	using Signal = typename ISignalling<R(Args...)>::Signal;
	Signal m_signal;
};
}  // namespace middleware
#endif  // MiddlewareSignalling_H
