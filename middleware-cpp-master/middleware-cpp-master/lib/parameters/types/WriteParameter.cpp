// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: WriteParameter.cpp
// Description: Interface API for external users.

#include "WriteParameter.h"
#include "MiddlewareFrameworkFactory.h"
#include "ParameterServiceStructures.h"
#include "SystemUtilities.h"

#include <middleware_parameter.pb.h>
#include <cassert>
#include <chrono>
#include <thread>

namespace middleware
{
namespace parameter
{
WriteParameter::WriteParameter(
    ::middleware::parameter::client::WriteParameter const& parameterConfig,
    std::shared_ptr<::middleware::IMiddlewareClient> const& spClient
)
    : m_identity{parameterConfig.parameter_id}, m_unit{parameterConfig.unit}, m_spClient(spClient)
{
	assert(m_spClient);
}

WriteParameter::~WriteParameter()
{
	// ensure destruction / shouldStop is set before any members of this class (which may be accessed by callbacks) are destructed
	m_spClient.reset();
};

void WriteParameter::Write(Data const& data, ::middleware::parameter::IWrite::OnCompletion&& callback)
{
	assert(callback);
	auto handler = [on_complete = std::move(callback), this](auto const& reply)
	{
		::middleware::parameter::IWrite::WriteStatus status{};
		status.status = ::middleware::parameter::Status::FAILURE;

		Parameter parameter{};
		parameter.mutable_info()->set_identity(m_identity);
		parameter.mutable_info()->set_unit(static_cast<uint32_t>(m_unit));
		parameter.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_NOT_RECEIVED);

		if(reply.status.status_code == ::middleware::StatusCode::STATUS_CODE_SUCCESS)
		{
			Parameter received{};
			if(parameter.mutable_data()->ParseFromArray(
			       reply.serialized_protobuf.data(), reply.serialized_protobuf.size()
			   ))
			{
				if(parameter.data().quality() == ::cat::middleware::parameter::QUALITY_GOOD)
				{
					status.status = ::middleware::parameter::Status::SUCCESS;
				}
				else
				{
					::middleware::log::Info("[WriteParameter] Reply to [%s] is not QUALITY_GOOD.", m_identity.c_str());
				}
			}
			else
			{
				::middleware::log::Warning("[WriteParameter] Reply to [%s] could not be parsed.", m_identity.c_str());
			}
		}
		else
		{
			::middleware::log::Warning(
			    "[WriteParameter] Reply for [%s] failed due to [%s].",
			    m_identity.c_str(),
			    reply.status.status_message.c_str()
			);
		}
		on_complete(status, parameter);
	};

	Parameter parameter{};
	parameter.mutable_info()->set_identity(m_identity);
	parameter.mutable_info()->set_unit(static_cast<uint32_t>(m_unit));
	parameter.mutable_data()->CopyFrom(data);

	m_spClient->Write(parameter, std::move(handler));
}
}  // namespace parameter
}  // namespace middleware
