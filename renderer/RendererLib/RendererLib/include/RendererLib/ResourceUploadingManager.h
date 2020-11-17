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
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class RendererResourceRegistry;
    class IResourceUploader;
    class IRenderBackend;
    struct RenderBuffer;
    class FrameTimer;
    class RendererStatistics;

    class ResourceUploadingManager
    {
    public:
        ResourceUploadingManager(
            RendererResourceRegistry& resources,
            IResourceUploader& uploader,
            IRenderBackend& renderBackend,
            Bool keepEffects,
            const FrameTimer& frameTimer,
            RendererStatistics& stats,
            UInt64 gpuCacheSize);
        ~ResourceUploadingManager();

        Bool hasAnythingToUpload() const;
        void uploadAndUnloadPendingResources();

        static const UInt32 NumResourcesToUploadInBetweenTimeBudgetChecks = 10u;
        static const UInt32 LargeResourceByteSizeThreshold = 250000u;

    private:
        void unloadResources(const ResourceContentHashVector& resourcesToUnload);
        void uploadResources(const ResourceContentHashVector& resourcesToUpload);
        void uploadResource(const ResourceDescriptor& rd);
        void unloadResource(const ResourceDescriptor& rd);
        void getResourcesToUnloadNext(ResourceContentHashVector& resourcesToUnload, Bool keepEffects, UInt64 sizeToBeFreed) const;
        void getAndPrepareResourcesToUploadNext(ResourceContentHashVector& resourcesToUpload, UInt64& totalSize) const;
        UInt64 getAmountOfMemoryToBeFreedForNewResources(UInt64 sizeToUpload) const;

        RendererResourceRegistry& m_resources;
        IResourceUploader&              m_uploader;
        IRenderBackend&                 m_renderBackend;

        const Bool   m_keepEffects;
        const FrameTimer& m_frameTimer;

        using SizeMap = HashMap<ResourceContentHash, UInt32>;
        SizeMap       m_resourceSizes;
        UInt64        m_resourceTotalUploadedSize = 0u;
        const UInt64  m_resourceCacheSize = 0u;

        RendererStatistics& m_stats;
    };
}

#endif
