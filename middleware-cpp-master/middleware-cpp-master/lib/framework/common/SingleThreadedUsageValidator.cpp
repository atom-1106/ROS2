// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SingleThreadedUsageValidator.cpp
// Description: Single Threaded Usage Validator

#include "SingleThreadedUsageValidator.h"
#include <cassert>

namespace middleware
{
#ifndef NDEBUG
[[gnu::weak]] void SingleThreadedUsageValidator::ValidateCurrentThread() const
{
	assert(m_threadId == GetId());
}

void LazySingleThreadedUsageValidator::ValidateCurrentThread() const
{
	if(!m_spSingleThreadedUsageValidator)
	{
		m_spSingleThreadedUsageValidator = std::make_unique<SingleThreadedUsageValidator>();
	}

	m_spSingleThreadedUsageValidator->ValidateCurrentThread();
}
#endif
}  // namespace middleware
