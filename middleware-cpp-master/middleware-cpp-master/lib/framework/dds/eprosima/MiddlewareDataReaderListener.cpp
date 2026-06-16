// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareDataReaderListener.cpp
// <<Non-Testable>>
// Description: DataReader Listener control class

#include "MiddlewareDataReaderListener.h"
#include "IMiddlewareReader.h"
#include "SystemUtilities.h"

// Standard
#include <cassert>

namespace middleware
{
namespace dds
{
MiddlewareDataReaderListener::MiddlewareDataReaderListener() : m_pMiddlewareReader{nullptr}
{
}
void MiddlewareDataReaderListener::SetDependency(IMiddlewareReader* pMiddlewareReader)
{
	assert(pMiddlewareReader != nullptr);
	assert(m_pMiddlewareReader == nullptr);
	m_pMiddlewareReader = pMiddlewareReader;
}
void MiddlewareDataReaderListener::on_subscription_matched(
    ::eprosima::fastdds::dds::DataReader*,
    ::eprosima::fastdds::dds::SubscriptionMatchedStatus const& info
)
{
	m_pMiddlewareReader->OnSubscriptionMatched(info.current_count_change > 0);
}
void MiddlewareDataReaderListener::on_data_available(::eprosima::fastdds::dds::DataReader*)
{
	m_pMiddlewareReader->OnDataAvailable();
}

}  // namespace dds
}  // namespace middleware
