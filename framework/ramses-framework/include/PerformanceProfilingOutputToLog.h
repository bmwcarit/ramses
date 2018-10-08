//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCEPROFILINGOUTPUTTOLOG_H
#define RAMSES_PERFORMANCEPROFILINGOUTPUTTOLOG_H

#include "Utils/LogMacros.h"

namespace ramses_internal
{
    class ScopedPerformanceProfilerRegionLog
    {
    public:
        ScopedPerformanceProfilerRegionLog(const char* regionName)
            : m_regionName(regionName)
        {
            LOG_TRACE(CONTEXT_PROFILING, m_regionName << " begin");
        }

        ~ScopedPerformanceProfilerRegionLog()
        {
            LOG_TRACE(CONTEXT_PROFILING, m_regionName << " end");
        }

    private:
        const char* m_regionName;
    };

#define PERFORMANCE_PROFILER_REGION_SCOPED(regionName) \
    ScopedPerformanceProfilerRegionLog ramsesScopedLogProfilerRegion##regionName(#regionName);

#define PERFORMANCE_PROFILER_FRAME_SCOPED(frameName)

}


#endif
