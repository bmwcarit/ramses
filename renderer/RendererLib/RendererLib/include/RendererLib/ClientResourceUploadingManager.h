//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTRESOURCEUPLOADINGMANAGER_H
#define RAMSES_CLIENTRESOURCEUPLOADINGMANAGER_H

#include "RendererLib/ResourceDescriptor.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class RendererClientResourceRegistry;
    class IResourceUploader;
    class IRenderBackend;
    struct RenderBuffer;
    class FrameTimer;
    class RendererStatistics;

    class ClientResourceUploadingManager
    {
    public:
        ClientResourceUploadingManager(
            RendererClientResourceRegistry& resources,
            IResourceUploader& uploader,
            IRenderBackend& renderBackend,
            Bool keepEffects,
            const FrameTimer& frameTimer,
            RendererStatistics& stats,
            UInt64 clientResourceCacheSize);
        ~ClientResourceUploadingManager();

        Bool hasAnythingToUpload() const;
        void uploadAndUnloadPendingResources();

        static const UInt32 NumResourcesToUploadInBetweenTimeBudgetChecks = 10u;
        static const UInt32 LargeResourceByteSizeThreshold = 250000u;

    private:
        void unloadClientResources(const ResourceContentHashVector& resourcesToUnload);
        void uploadClientResources(const ResourceContentHashVector& resourcesToUpload);
        void uploadClientResource(const ResourceDescriptor& rd);
        void unloadClientResource(const ResourceDescriptor& rd);
        void getClientResourcesToUnloadNext(ResourceContentHashVector& resourcesToUnload, Bool keepEffects, UInt64 sizeToBeFreed) const;
        void getAndPrepareClientResourcesToUploadNext(ResourceContentHashVector& resourcesToUpload, UInt64& totalSize) const;
        UInt64 getAmountOfMemoryToBeFreedForNewResources(UInt64 sizeToUpload) const;

        RendererClientResourceRegistry& m_clientResources;
        IResourceUploader&              m_uploader;
        IRenderBackend&                 m_renderBackend;

        const Bool   m_keepEffects;
        const FrameTimer& m_frameTimer;

        using SizeMap = HashMap<ResourceContentHash, UInt32>;
        SizeMap       m_clientResourceSizes;
        UInt64        m_clientResourceTotalUploadedSize = 0u;
        const UInt64  m_clientResourceCacheSize = 0u;

        RendererStatistics& m_stats;
    };
}

#endif
