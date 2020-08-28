//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererLib/MemoryStatistics.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererResourceManager.h"
#include "Utils/StatisticCollection.h"


namespace ramses_internal
{

    MemoryStatistics::MemoryStatistics()
    {
        m_memorySamples.reserve(100);
    }

    void MemoryStatistics::addMemorySample(const GpuMemorySample& memorySample)
    {
        m_memorySamples.push_back(memorySample);
    }

    void MemoryStatistics::writeMemoryUsageSummaryToString(StringOutputStream& str) const
    {
        // Early exit if memory statistics are disabled
        if (m_memorySamples.empty())
        {
            return;
        }

        SummaryEntry<UInt64> memoryUsageSummaryMB;
        SummaryEntry<UInt64> memoryUsageTexturesMB;
        SummaryEntry<UInt64> memoryUsageGeometryMB;
        SummaryEntry<UInt64> memoryUsageRenderBuffersMB;
        HashMap<SceneId, SummaryEntry<UInt64>> memoryUsagePerScene;

        for (const auto& memSample : m_memorySamples)
        {
            memoryUsageSummaryMB.update(memSample.getTotalMemoryUsage() >> 20);
            memoryUsageTexturesMB.update(memSample.getTextureMemoryUsage() >> 20);
            memoryUsageGeometryMB.update(memSample.getGeometryMemoryUsage() >> 20);
            memoryUsageRenderBuffersMB.update(memSample.getRenderbufferMemoryUsage() >> 20);

            const SceneIdVector& sampledScenes = memSample.getSampledScenes();
            for (const auto& scene : sampledScenes)
            {
                memoryUsagePerScene[scene].update(memSample.getTotalSceneMemoryUsage(scene) >> 20);
            }
        }

        const UInt64 currentMemUsageMBytes = getCurrentMemoryUsageInMBytes();

        str << "MemUsgMB (Cur:" << currentMemUsageMBytes << ";Avg:" << memoryUsageSummaryMB.sum / m_memorySamples.size() << ";Min:" << memoryUsageSummaryMB.minValue << ";Max:" << memoryUsageSummaryMB.maxValue << "); ";
        str << "Tex (Avg:" << memoryUsageTexturesMB.sum / m_memorySamples.size() << ";Min:" << memoryUsageTexturesMB.minValue << ";Max:" << memoryUsageTexturesMB.maxValue << "); ";
        str << "Geom (Avg:" << memoryUsageGeometryMB.sum / m_memorySamples.size() << ";Min:" << memoryUsageGeometryMB.minValue << ";Max:" << memoryUsageGeometryMB.maxValue << "); ";
        str << "RendBuf (Avg:" << memoryUsageRenderBuffersMB.sum / m_memorySamples.size() << ";Min:" << memoryUsageRenderBuffersMB.minValue << ";Max:" << memoryUsageRenderBuffersMB.maxValue << "); ";
        for (const auto& scene : memoryUsagePerScene)
        {
            const SummaryEntry<UInt64>& sceneMemorySample = scene.value;
            str << "; Scene " << scene.key << " (Avg:" << sceneMemorySample.sum / m_memorySamples.size() << ";Min:" << sceneMemorySample.minValue << ";Max:" << sceneMemorySample.maxValue << ")";
        }
    }

    UInt64 MemoryStatistics::getCurrentMemoryUsageInMBytes() const
    {
        if (!m_memorySamples.empty())
        {
            return m_memorySamples.back().getTotalMemoryUsage() >> 20;
        }
        return 0u;
    }

    void MemoryStatistics::reset()
    {
        m_memorySamples.clear();
    }
}
