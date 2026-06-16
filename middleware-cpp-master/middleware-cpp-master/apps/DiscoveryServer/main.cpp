// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Middleware Discovery Server

#include <condition_variable>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <mutex>

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
	::eprosima::fastdds::rtps::Locator_t locator;
	locator.kind = LOCATOR_KIND_UDPv4;
#if BUILD_TARGET == TGT_GCC_ARM64_GUEST
	::eprosima::fastdds::rtps::IPLocator::setIPv4(locator, 0, 0, 0, 0);
#elif BUILD_TARGET == TGT_GCC_X86_WORKSTATION
	::eprosima::fastdds::rtps::IPLocator::setIPv4(locator, 127, 0, 0, 1);
#else
#error "Unexpected target in DiscoveryServer app"
#endif
	locator.port = 64863;
	::eprosima::fastdds::dds::DomainParticipantQos participantQos;
	participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol =
	    ::eprosima::fastdds::rtps::DiscoveryProtocol::SERVER;
	participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
	::eprosima::fastdds::dds::DomainParticipantListener listener{};
	::eprosima::fastdds::dds::DomainParticipant* m_pDomainParticipant =
	    ::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
	        0, participantQos, &listener
	    );
	(void)m_pDomainParticipant;

	std::mutex mutex{};
	std::condition_variable condition{};
	std::unique_lock<std::mutex> lock{mutex};
	condition.wait(lock, [] { return false; });
	return 0;
}
