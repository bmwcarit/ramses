//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEUPLOADINGMANAGER_H
#define RAMSES_RESOURCEUPLOADINGMANAGER_H

#include "RendererLib/ResourceDescriptor.h"
#include "RendererLib/IResourceUploader.h"
#include "RendererLib/AsyncEffectUploader.h"
#include "Collections/HashMap.h"
#include <map>

namespace ramses_internal
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

        [[nodiscard]] UInt32 getResourceUploadBatchSize() const
        {
            return m_resourceUploadBatchSize;
        }

        static const UInt32 LargeResourceByteSizeThreshold = 250000u;

    private:
        void unloadResources(const ResourceContentHashVector& resourcesToUnload);
        void uploadResources(const ResourceContentHashVector& resourcesToUpload);
        void syncEffects();
        void uploadResource(const ResourceDescriptor& rd);
        void unloadResource(const ResourceDescriptor& rd);
        void getResourcesToUnloadNext(ResourceContentHashVector& resourcesToUnload, bool keepEffects, UInt64 sizeToBeFreed) const;
        void getAndPrepareResourcesToUploadNext(ResourceContentHashVector& resourcesToUpload, UInt64& totalSize) const;
        [[nodiscard]] Int32 getScenePriority(const ResourceDescriptor& rd) const;
        [[nodiscard]] UInt64 getAmountOfMemoryToBeFreedForNewResources(UInt64 sizeToUpload) const;

        RendererResourceRegistry& m_resources;
        std::unique_ptr<IResourceUploader> m_uploader;
        IRenderBackend&                 m_renderBackend;
        AsyncEffectUploader&            m_asyncEffectUploader;
        EffectsRawResources             m_effectsToUpload;
        EffectsGpuResources             m_effectsUploadedTemp; //to avoid re-allocation each frame

        const bool   m_keepEffects;
        const FrameTimer& m_frameTimer;

        using SizeMap = HashMap<ResourceContentHash, UInt32>;
        SizeMap       m_resourceSizes;
        UInt64        m_resourceTotalUploadedSize = 0u;
        const UInt64  m_resourceCacheSize = 0u;
        const UInt32  m_resourceUploadBatchSize   = 10u;

        RendererStatistics& m_stats;

        std::unordered_map<SceneId, int32_t> m_scenePriorities;
        mutable std::map<int32_t, ResourceContentHashVector> m_buckets;
    };
}

#endif
