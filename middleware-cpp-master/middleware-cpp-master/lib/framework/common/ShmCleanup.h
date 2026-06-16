// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ShmCleanup.h
// Description: Cleans up things in shared memory left behind by Fast-DDS when not torn down.

#ifndef ShmCleanup_H
#define ShmCleanup_H

#include <string>

namespace middleware
{
void ShmCleanup(std::string const& dir = "/dev/shm");
}  // namespace middleware

#endif  // ShmCleanup_H
