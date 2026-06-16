// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ISignalling.h
// Description: Boost::signals interface handler.

#ifndef MiddlewareInterfaceSignalling_H
#define MiddlewareInterfaceSignalling_H

#include <boost/signals2.hpp>
#include <functional>
#include <memory>
#include "ISignalConnection.h"

namespace middleware
{
// Yes, this forward declaration is necessary to do the std::function<> like callback style
template <class R, class... Args>
class ISignalling;

template <class R, class... Args>
class ISignalling<R(Args...)>
{
public:
	using Callback        = R(Args...);
	using Signal          = boost::signals2::signal<Callback>;
	using Registration    = std::unique_ptr<::middleware::ISignalConnection>;
	using GroupType       = typename boost::signals2::signal<Callback>::group_type;
	using ConnectPosition = boost::signals2::connect_position;

	static constexpr GroupType default_group                  = 0;
	static constexpr ConnectPosition default_connect_position = ConnectPosition::at_back;

	virtual ~ISignalling() = default;

	/// Connect to an event by ID.
	virtual Registration Register(
	    std::function<Callback> const& callback,
	    GroupType const group                     = ISignalling<ISignalling::Callback>::default_group,
	    ConnectPosition const connection_position = ISignalling<ISignalling::Callback>::default_connect_position
	) = 0;

	/// Disconnect from an event.
	virtual void Deregister(Registration& registration) = 0;
};
}  // namespace middleware
#endif  // MiddlewareInterfaceSignalling_H
