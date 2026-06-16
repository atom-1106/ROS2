// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ReadParameter.h
// Description: Interface API for external users.

#ifndef ReadParameter_H
#define ReadParameter_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include "IReadParameter.h"

namespace middleware
{
namespace parameter
{
namespace client
{
struct ReadParameter;
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
class ReadParameter final : public ::middleware::parameter::IRead
{
public:
	ReadParameter(
	    ::middleware::parameter::client::ReadParameter const& parameterConfig,
	    std::shared_ptr<::middleware::IMiddlewareClient> const& spClient
	);
	~ReadParameter() override;
	void Read(::middleware::parameter::IRead::OnRequest&& callback) override;
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
#endif  // ReadParameter_H
