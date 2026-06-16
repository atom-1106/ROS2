// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DomainParticipantMessageStub.cpp
// <<Non-Testable>>
// Description: Stub for third-party library content control.

#include "DomainParticipantMessageStub.h"
#include "MessageReader.h"
#include "MessageWriter.h"
#include "SystemUtilities.h"

#include "messagePubSubTypes.hpp"

// Fast DDS
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPLocator.hpp>

// Standard
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static inline void ApplyDiscoveryServer(::eprosima::fastdds::dds::DomainParticipantQos& qos)
{
	::eprosima::fastdds::rtps::Locator_t locator;
	locator.kind = LOCATOR_KIND_UDPv4;
	locator.port = 64863;
	std::vector<std::array<unsigned int, 4>> vmIPs{};
#if BUILD_TARGET == TGT_GCC_ARM64_GUEST
	// temporary hack - we run the Discovery Server on the lowest VM IP
	std::ifstream hosts{"/etc/hosts"};
	std::string line;
	while(std::getline(hosts, line))
	{
		if(!line.starts_with("172."))
		{
			continue;
		}
		std::for_each(line.begin(), line.end(), static_cast<int (*)(int)>(std::tolower));
		bool skip        = false;
		size_t hostIndex = 0;
		// clang-format off
		// cppcheck-suppress stlIfStrFind
		while(hostIndex = line.find("host", hostIndex + 1), hostIndex != std::string::npos)
		{
			if(line[hostIndex - 1] < 'a' || line[hostIndex - 1] > 'z') { skip = true; break; }
			hostIndex += std::strlen("host");
			if(hostIndex >= line.length()) { break; }
			if(line[hostIndex] < 'a' || line[hostIndex] > 'z') { skip = true; break; }
		}
		if(skip) { continue; }
		std::array<unsigned int, 4> ip{0, 0, 0, 0};
		std::istringstream lineStream{line};
		lineStream >> ip[0]; lineStream.get();
		lineStream >> ip[1]; lineStream.get();
		lineStream >> ip[2]; lineStream.get();
		lineStream >> ip[3];
		// clang-format on
		vmIPs.push_back(std::move(ip));
	}
	if(vmIPs.size() == 0)
		return;
#elif BUILD_TARGET == TGT_GCC_X86_WORKSTATION
	vmIPs.push_back({127, 0, 0, 1});
#else
#error "Unexpected target in ApplyDiscoveryServer"
#endif
	std::sort(vmIPs.begin(), vmIPs.end());
	::eprosima::fastdds::rtps::IPLocator::setIPv4(locator, vmIPs[0][0], vmIPs[0][1], vmIPs[0][2], vmIPs[0][3]);
	qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
	    ::eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;
	qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(locator);
}

namespace middleware
{
namespace dds
{
DomainParticipantMessageStub::DomainParticipantMessageStub(uint32_t const domain_id, std::string const& name)
    : m_name(name),
      m_topicSupportType(new ::cat::middleware::MessagePubSubType()),
      m_pPublisher{nullptr},
      m_pSubscriber{nullptr}
{
	auto qos = ::eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
	qos.name(m_name);
	qos.wire_protocol().builtin.avoid_builtin_multicast = false;
	// we cannot run a Discovery Server automatically for x86
	// automatically running one with the Middleware verity for arm64 has not been tested all-together
	// even then, people using a Middleware verity to get the library is not common yet
	// -> don't use the discovery server for now
	//ApplyDiscoveryServer(qos);

	auto spParticipantFactory = ::eprosima::fastdds::dds::DomainParticipantFactory::get_shared_instance();
	// Create DomainParticipant
	m_pDomainParticipant = spParticipantFactory->create_participant(domain_id, qos);
	assert(m_pDomainParticipant);
	m_topicSupportType.register_type(m_pDomainParticipant);

	::middleware::log::Info("[DomainParticipantMessageStub] %s is created.", m_name.c_str());
}
DomainParticipantMessageStub::~DomainParticipantMessageStub()
{
	::middleware::log::Debug("[DomainParticipantMessageStub] [%s] destructor called.", m_name.c_str());
	if(m_pPublisher != nullptr)
	{
		::middleware::log::Debug("[DomainParticipantMessageStub] [%s] called delete_publisher.", m_name.c_str());
		m_pDomainParticipant->delete_publisher(m_pPublisher);
	}
	if(m_pSubscriber != nullptr)
	{
		::middleware::log::Debug("[DomainParticipantMessageStub] [%s] called delete_subscriber.", m_name.c_str());
		m_pDomainParticipant->delete_subscriber(m_pSubscriber);
	}
	::middleware::log::Info("[DomainParticipantMessageStub] [%s] delete_contained_entities", m_name.c_str());
	auto spParticipantFactory = ::eprosima::fastdds::dds::DomainParticipantFactory::get_shared_instance();
	spParticipantFactory->delete_participant(m_pDomainParticipant);
	::middleware::log::Debug("[DomainParticipantMessageStub] [%s] Fast DDS clean complete.", m_name.c_str());
}
uint32_t DomainParticipantMessageStub::MulticastListeningLocatorPort()
{
	// https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/listening_locators.html#well-known-ports
	auto const portBase     = m_pDomainParticipant->get_qos().wire_protocol().port.portBase;
	auto const domainIDGain = m_pDomainParticipant->get_qos().wire_protocol().port.domainIDGain;
	auto const offsetd2     = m_pDomainParticipant->get_qos().wire_protocol().port.offsetd2;
	return (portBase + domainIDGain * m_pDomainParticipant->get_domain_id() + offsetd2);
}
std::unique_ptr<::middleware::IMiddlewareReader> DomainParticipantMessageStub::CreateReader(
    std::string const& topic_name,
    ::eprosima::fastdds::dds::DataReaderQos const& qos,
    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
)
{
	if(m_pSubscriber == nullptr)
	{
		m_pSubscriber = m_pDomainParticipant->create_subscriber(::eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
		assert(m_pSubscriber);
	}

	return ::middleware::dds::MessageReader::Create(
	    topic_name,
	    m_pDomainParticipant,
	    m_pSubscriber,
	    m_topicSupportType,
	    qos,
	    std::move(on_subscription),
	    std::move(on_disconnect)
	);
}
std::unique_ptr<::middleware::IMiddlewareWriter> DomainParticipantMessageStub::CreateWriter(
    std::string const& topic_name,
    ::eprosima::fastdds::dds::DataWriterQos const& qos
)
{
	if(m_pPublisher == nullptr)
	{
		m_pPublisher = m_pDomainParticipant->create_publisher(::eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
		assert(m_pPublisher);
	}

	return ::middleware::dds::MessageWriter::Create(
	    topic_name, m_pDomainParticipant, m_pPublisher, m_topicSupportType, qos
	);
}
}  // namespace dds
}  // namespace middleware
