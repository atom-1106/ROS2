// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareDataWriterListener.cpp
// <<Non-Testable>>
// Description: DataWriter Listener control class

#include "MiddlewareDataWriterListener.h"
#include "IMiddlewareWriter.h"
#include "SystemUtilities.h"

// Standard
#include <cassert>

namespace middleware
{
namespace dds
{
MiddlewareDataWriterListener::MiddlewareDataWriterListener() : m_pMiddlewareWriter{nullptr}
{
}
void MiddlewareDataWriterListener::SetDependency(IMiddlewareWriter* pMiddlewareWriter)
{
	assert(pMiddlewareWriter != nullptr);
	assert(m_pMiddlewareWriter == nullptr);
	m_pMiddlewareWriter = pMiddlewareWriter;
}
void MiddlewareDataWriterListener::on_publication_matched(
    ::eprosima::fastdds::dds::DataWriter*,
    ::eprosima::fastdds::dds::PublicationMatchedStatus const& info
)
{
	m_pMiddlewareWriter->OnPublicationMatched(info.current_count_change > 0);
}

}  // namespace dds
}  // namespace middleware
