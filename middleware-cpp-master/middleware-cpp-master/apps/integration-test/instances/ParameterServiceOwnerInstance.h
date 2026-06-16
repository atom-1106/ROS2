// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceOwnerInstance.h
// Description: Integration Owner Instance Handler Helper Class

#ifndef ParameterServiceOwnerInstance_H
#define ParameterServiceOwnerInstance_H

#include <IBasicParameter.h>
#include <IConfigBuilder.h>
#include <IConfigTaker.h>
#include <IParameterService.h>
#include <IPublishParameter.h>
#include <IReadParameter.h>
#include <IWriteParameter.h>
#include <ParameterServiceDefines.h>
#include <ParameterServiceFactory.h>
#include <middleware_parameter.pb.h>

#include <condition_variable>
#include <memory>
#include <queue>
#include <string>
#include <thread>

class ParameterServiceOwnerInstance
{
public:
	ParameterServiceOwnerInstance(std::string const& name);
	virtual ~ParameterServiceOwnerInstance();
	void Stop();
	void AddPublisher(
	    std::string const& name,
	    std::string const& initialValue,
	    ::cat::middleware::parameter::Quality const initialQuality
	);
	void AddPublisher(
	    std::string const& name,
	    std::string const& initialValue,
	    ::middleware::parameter::Unit const unit,
	    ::cat::middleware::parameter::Quality const initialQuality
	);
	void AddOnWrite(std::string const& name, ::cat::middleware::parameter::Quality const returnQuality);
	void AddOnRead(
	    std::string const& name,
	    std::string const& returnValue,
	    ::cat::middleware::parameter::Quality const returnQuality
	);
	void
	NextValue(std::string const& name, std::string const& value, ::cat::middleware::parameter::Quality const quality);
	void Run();

private:
	struct NextValueType
	{
		std::string name{};
		::middleware::parameter::Data data{};
	};

	std::unique_ptr<::middleware::parameter::IConfigBuilder> m_spConfiguration;

	std::mutex m_mutex{};
	std::condition_variable m_condition{};
	std::thread m_thread{};
	std::atomic_bool m_run{false};
	std::atomic_bool m_wakeup{false};
	std::queue<NextValueType> m_nextValueToPublishQueue{};
};
#endif  // ParameterServiceOwnerInstance_H
