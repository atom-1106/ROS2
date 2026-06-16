// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareDataReaderListener.h
// <<Non-Testable>>
// Description: DataReader Listener control class

#ifndef MiddlewareDataReaderListener_H
#define MiddlewareDataReaderListener_H

#include <fastdds/dds/subscriber/DataReaderListener.hpp>

namespace middleware
{
class IMiddlewareReader;

namespace dds
{
class MiddlewareDataReaderListener final : public ::eprosima::fastdds::dds::DataReaderListener
{
public:
	MiddlewareDataReaderListener();
	virtual ~MiddlewareDataReaderListener() override = default;
	void SetDependency(::middleware::IMiddlewareReader* pMiddlewareReader);

	void on_subscription_matched(
	    ::eprosima::fastdds::dds::DataReader* pReader,
	    ::eprosima::fastdds::dds::SubscriptionMatchedStatus const& info
	) override;
	void on_data_available(::eprosima::fastdds::dds::DataReader* pReader) override;

private:
	::middleware::IMiddlewareReader* m_pMiddlewareReader;
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareDataReaderListener_H
