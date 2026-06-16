// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: WriteParameter.h
// Description: Interface API for external users.

#ifndef WriteParameter_H
#define WriteParameter_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include "IWriteParameter.h"

namespace middleware
{
namespace parameter
{
namespace client
{
struct WriteParameter;
}
}  // namespace parameter
}  // namespace middleware

namespace middleware
{
class IMiddlewareClient;
}  // namespace middleware

namespace middleware
{
namespace parameter
{
class WriteParameter final : public ::middleware::parameter::IWrite
{
public:
	WriteParameter(
	    ::middleware::parameter::client::WriteParameter const& parameterConfig,
	    std::shared_ptr<::middleware::IMiddlewareClient> const& spClient
	);
	~WriteParameter() override;
	void Write(Data const& data, ::middleware::parameter::IWrite::OnCompletion&& callback) override;
	::middleware::parameter::Identity Identity() override
	{
		return m_identity;
	}
	::middleware::parameter::Unit Unit() override
	{
		return m_unit;
	}

private:
	::middleware::parameter::Identity const m_identity{};
	::middleware::parameter::Unit const m_unit{};
	std::shared_ptr<::middleware::IMiddlewareClient> m_spClient;
};
}  // namespace parameter
}  // namespace middleware
#endif  // WriteParameter_H
