//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYSTATISTICS_H
#define RAMSES_MEMORYSTATISTICS_H

#include "Collections/Vector.h"
#include "SceneAPI/SceneId.h"
#include "Resource/EResourceType.h"
#include "Collections/HashMap.h"
#include "GpuMemorySample.h"

namespace ramses_internal
{
    class RendererSceneUpdater;

    class MemoryStatistics
    {
    public:
        MemoryStatistics();

        void addMemorySample(const GpuMemorySample& memorySample);
        void writeMemoryUsageSummaryToString(StringOutputStream& str) const;
        void reset();

    private:
        UInt64 getCurrentMemoryUsageInMBytes() const;

        std::vector<GpuMemorySample> m_memorySamples;
    };
}

#endif
