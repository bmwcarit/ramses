//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCEPROFILINGOUTPUTTOVTUNE_H
#define RAMSES_PERFORMANCEPROFILINGOUTPUTTOVTUNE_H

#include <ittnotify.h>

namespace ramses_internal
{
    class PerformanceProfilerUtilsVTune
    {
    public:
        static const __itt_domain* GetDomain()
        {
            static const __itt_domain* sDomain = __itt_domain_create("RAMSES");
            return sDomain;
        }

    };

    class ScopedPerformanceProfilerRegionVTune
    {
    public:
        ScopedPerformanceProfilerRegionVTune(__itt_string_handle* regionHandle)
        {
            __itt_task_begin(PerformanceProfilerUtilsVTune::GetDomain(), __itt_null, __itt_null, regionHandle);
        }

        ~ScopedPerformanceProfilerRegionVTune()
        {
            __itt_task_end(PerformanceProfilerUtilsVTune::GetDomain());
        }

    };

    class ScopedPerformanceProfilerFrameVTune
    {
    public:
        ScopedPerformanceProfilerFrameVTune(void* data, unsigned long long extra)
        {
            mItt_id = __itt_id_make(data, extra);
            __itt_frame_begin_v3(PerformanceProfilerUtilsVTune::GetDomain(), &mItt_id);
        }

        ~ScopedPerformanceProfilerFrameVTune()
        {
            __itt_frame_end_v3(PerformanceProfilerUtilsVTune::GetDomain(), &mItt_id);
        }

    private:
        __itt_id mItt_id = __itt_null;

    };

#define PERFORMANCE_PROFILER_REGION_SCOPED(regionName) \
    static __itt_string_handle* region##regionName = __itt_string_handle_create(#regionName); \
    ScopedPerformanceProfilerRegionVTune ramsesScopedPerformanceProfilerRegion##regionName(region##regionName);

#define PERFORMANCE_PROFILER_FRAME_SCOPED(frameName) \
    static __itt_string_handle* frame##frameName = __itt_string_handle_create(#frameName); \
    ScopedPerformanceProfilerFrameVTune ramsesScopedPerformanceProfilerFrame##frameName(static_cast<void*>(frame##frameName), 0);
}

#endif
