/******************************************************************************************************************
 Copyright 2021 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: TimeDiffNS.h

Description:  calculate ns diff from 2x timespace samples

********************************************************************************************************************/
#ifndef TIME_DIFF_NS
#define TIME_DIFF_NS

uint64_t TimeDiffNS(struct timespec newtime, struct timespec oldtime);

#endif
