// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareDataWriterListener.h
// <<Non-Testable>>
// Description: DataWriter Listener control class

#ifndef MiddlewareDataWriterListener_H
#define MiddlewareDataWriterListener_H

#include <fastdds/dds/publisher/DataWriterListener.hpp>

namespace middleware
{
class IMiddlewareWriter;

namespace dds
{
class MiddlewareDataWriterListener final : public ::eprosima::fastdds::dds::DataWriterListener
{
public:
	MiddlewareDataWriterListener();
	virtual ~MiddlewareDataWriterListener() override = default;
	void SetDependency(::middleware::IMiddlewareWriter* pMiddlewareWriter);
	void on_publication_matched(
	    ::eprosima::fastdds::dds::DataWriter* spWriter,
	    ::eprosima::fastdds::dds::PublicationMatchedStatus const& info
	) override;

private:
	::middleware::IMiddlewareWriter* m_pMiddlewareWriter;
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareDataWriterListener_H
