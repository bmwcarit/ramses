//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCEPROFILING_H
#define RAMSES_PERFORMANCEPROFILING_H

#ifdef PERFORMANCE_PROFILER_OUTPUT_TYPE_LOG
#include "PerformanceProfilingOutputToLog.h"
#elif PERFORMANCE_PROFILER_OUTPUT_TYPE_VTUNE
#include "PerformanceProfilingOutputToVTune.h"
#else
#define PERFORMANCE_PROFILER_REGION_SCOPED(regionName)
#define PERFORMANCE_PROFILER_FRAME_SCOPED(frameName)
#endif

#endif
