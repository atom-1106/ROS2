// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Actions.h
// Description: Action handlers for middleware-parameters-tester.

#pragma once

#include "Args.h"

namespace mpt
{
/// @brief Publishes a single value then waits for the configured delay before exiting.
void RunPub(Args const& args);

/// @brief Subscribes to a parameter and logs every received update to stdout. Runs forever.
void RunSub(Args const& args);

/// @brief Sends a single RPC read request and exits after receiving the response.
void RunRead(Args const& args);

/// @brief Sends a single RPC write request and exits after receiving the response.
void RunWrite(Args const& args);
}  // namespace mpt
