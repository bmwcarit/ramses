//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/ResourceDescriptor.h"
#include "internal/RendererLib/IResourceUploader.h"
#include "internal/RendererLib/AsyncEffectUploader.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include <map>

namespace ramses::internal
{
    class RendererResourceRegistry;
    class IRenderBackend;
    struct RenderBuffer;
    class FrameTimer;
    class RendererStatistics;
    class DisplayConfig;

    class ResourceUploadingManager
    {
    public:
        ResourceUploadingManager(
            RendererResourceRegistry& resources,
            std::unique_ptr<IResourceUploader> uploader,
            IRenderBackend& renderBackend,
            AsyncEffectUploader& asyncEffectUploader,
            const DisplayConfig& displayConfig,
            const FrameTimer& frameTimer,
            RendererStatistics& stats);
        ~ResourceUploadingManager();

        [[nodiscard]] bool hasAnythingToUpload() const;
        void uploadAndUnloadPendingResources();

        [[nodiscard]] uint32_t getResourceUploadBatchSize() const
        {
            return m_resourceUploadBatchSize;
        }

        static const uint32_t LargeResourceByteSizeThreshold = 250000u;

    private:
        void unloadResources(const ResourceContentHashVector& resourcesToUnload);
        void uploadResources(const ResourceContentHashVector& resourcesToUpload);
        void syncEffects();
        void uploadResource(const ResourceDescriptor& rd);
        void unloadResource(const ResourceDescriptor& rd);
        void getResourcesToUnloadNext(ResourceContentHashVector& resourcesToUnload, uint64_t sizeToBeFreed, bool keepEffects = true) const;
        void getAndPrepareResourcesToUploadNext(ResourceContentHashVector& resourcesToUpload, uint64_t& totalSize) const;
        [[nodiscard]] int32_t getScenePriority(const ResourceDescriptor& rd) const;
        [[nodiscard]] uint64_t getAmountOfMemoryToBeFreedForNewResources(uint64_t sizeToUpload) const;

        RendererResourceRegistry& m_resources;
        std::unique_ptr<IResourceUploader> m_uploader;
        IRenderBackend&                 m_renderBackend;
        AsyncEffectUploader&            m_asyncEffectUploader;
        EffectsRawResources             m_effectsToUpload;
        EffectsGpuResources             m_effectsUploadedTemp; //to avoid re-allocation each frame

        const FrameTimer& m_frameTimer;

        using SizeMap = HashMap<ResourceContentHash, uint32_t>;
        SizeMap       m_resourceSizes;
        uint64_t        m_resourceTotalUploadedSize = 0u;
        const uint64_t  m_resourceCacheSize = 0u;
        const uint32_t  m_resourceUploadBatchSize   = 10u;

        RendererStatistics& m_stats;

        std::unordered_map<SceneId, int32_t> m_scenePriorities;
        mutable std::map<int32_t, ResourceContentHashVector> m_buckets;
    };
}
