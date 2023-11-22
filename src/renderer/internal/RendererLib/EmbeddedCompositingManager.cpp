//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/EmbeddedCompositingManager.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "internal/RendererLib/PlatformInterface/ITextureUploadingAdapter.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    EmbeddedCompositingManager::EmbeddedCompositingManager(IDevice &device, IEmbeddedCompositor &embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter)
        : m_device(device)
        , m_embeddedCompositor(embeddedCompositor)
        , m_textureUploadingAdapter(textureUploadingAdapter)
    {
    }

    void EmbeddedCompositingManager::refStream(WaylandIviSurfaceId source)
    {
        auto streamTextureInfoIt = m_streamTextureSourceInfoMap.find(source);
        if (m_streamTextureSourceInfoMap.end() == streamTextureInfoIt)
        {
            LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingManager::refStream: Creating new stream texture with: {}", source);
            createStreamTexture(source);
            streamTextureInfoIt = m_streamTextureSourceInfoMap.find(source);
        }

        streamTextureInfoIt->value.refs++;
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingManager::refStream adding reference to stream texture {}. Total refs: {}", source, streamTextureInfoIt->value.refs);
    }

    void EmbeddedCompositingManager::unrefStream(WaylandIviSurfaceId source)
    {
        StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(source);
        assert(streamTextureSourceInfo != nullptr);

        // remove stream buffer reference
        assert(streamTextureSourceInfo->refs > 0);
        streamTextureSourceInfo->refs--;
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingManager::unrefStream removing reference to stream texture {}. Total refs: {}", source, streamTextureSourceInfo->refs);

        if (streamTextureSourceInfo->refs == 0)
        {
            LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingManager::unrefStream: Destroying no more referenced stream texture with {}", source);
            destroyStreamTexture(source);
        }
    }

    void EmbeddedCompositingManager::dispatchStateChangesOfSources(WaylandIviSurfaceIdVector& streamsWithAvailabilityChanged, WaylandIviSurfaceIdVector& newStreams, WaylandIviSurfaceIdVector& obsoleteStreams)
    {
        assert(newStreams.empty());
        assert(obsoleteStreams.empty());
        assert(streamsWithAvailabilityChanged.empty());

        const auto newStreamsSet = m_embeddedCompositor.dispatchNewStreamTextureSourceIds();
        const auto obsoleteStreamsSet = m_embeddedCompositor.dispatchObsoleteStreamTextureSourceIds();

        newStreams.resize(newStreamsSet.size());
        std::copy(newStreamsSet.cbegin(), newStreamsSet.cend(), newStreams.begin());
        obsoleteStreams.resize(obsoleteStreamsSet.size());
        std::copy(obsoleteStreamsSet.cbegin(), obsoleteStreamsSet.cend(), obsoleteStreams.begin());

        for (auto& streamTexture : m_streamTextureSourceInfoMap)
        {
            const bool contentAvailable = m_embeddedCompositor.isContentAvailableForStreamTexture(streamTexture.key);
            if (streamTexture.value.contentAvailable != contentAvailable)
            {
                streamsWithAvailabilityChanged.push_back(streamTexture.key);
                streamTexture.value.contentAvailable = contentAvailable;
            }
        }
    }

    DeviceResourceHandle EmbeddedCompositingManager::getCompositedTextureDeviceHandleForStreamTexture(WaylandIviSurfaceId source) const
    {
        if(m_embeddedCompositor.isContentAvailableForStreamTexture(source))
        {
            StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(source);
            assert(nullptr != streamTextureSourceInfo);
            return streamTextureSourceInfo->compositedTextureHandle;
        }

        return DeviceResourceHandle::Invalid();
    }

    bool EmbeddedCompositingManager::hasRealCompositor() const
    {
        return m_embeddedCompositor.isRealCompositor();
    }

    void EmbeddedCompositingManager::processClientRequests()
    {
        m_embeddedCompositor.handleRequestsFromClients();
        m_embeddedCompositor.endFrame(false);
    }

    bool EmbeddedCompositingManager::hasUpdatedContentFromStreamSourcesToUpload() const
    {
        return m_embeddedCompositor.hasUpdatedStreamTextureSources();
    }

    void EmbeddedCompositingManager::uploadResourcesAndGetUpdates(StreamSourceUpdates& updatedStreams)
    {
        const WaylandIviSurfaceIdSet updatedStreamTextureSourceIds = m_embeddedCompositor.dispatchUpdatedStreamTextureSourceIds();
        assert(updatedStreams.empty());
        updatedStreams.reserve(updatedStreamTextureSourceIds.size());

        for (const auto streamTextureSourceId : updatedStreamTextureSourceIds)
        {
            const StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(streamTextureSourceId);
            if (nullptr != streamTextureSourceInfo)
            {
                const uint32_t bufferUpdateCount = m_embeddedCompositor.uploadCompositingContentForStreamTexture(streamTextureSourceId, streamTextureSourceInfo->compositedTextureHandle, m_textureUploadingAdapter);
                updatedStreams.push_back({ streamTextureSourceId, bufferUpdateCount });
            }
        }
    }

    void EmbeddedCompositingManager::notifyClients()
    {
        m_embeddedCompositor.endFrame(true);
    }

    void EmbeddedCompositingManager::createStreamTexture(WaylandIviSurfaceId source)
    {
        // register a new texture (with dummy description and no data) to device and put entry in hashmap
        // actual data is uploaded later when source content available
        const DeviceResourceHandle compositedTextureDeviceHandle = m_device.uploadStreamTexture2D(DeviceResourceHandle::Invalid(), 1u, 1u, EPixelStorageFormat::RGBA8, nullptr, {ETextureChannelColor::Blue, ETextureChannelColor::Green, ETextureChannelColor::Red, ETextureChannelColor::Alpha});

        StreamTextureSourceInfo streamTextureSourceInfo;
        streamTextureSourceInfo.compositedTextureHandle = compositedTextureDeviceHandle;
        streamTextureSourceInfo.contentAvailable = false;
        m_streamTextureSourceInfoMap.put(source, streamTextureSourceInfo);

        if (m_embeddedCompositor.isContentAvailableForStreamTexture(source))
        {
            LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositingManager::uploadStreamTexture Content available for stream texture {}", source);
            m_embeddedCompositor.uploadCompositingContentForStreamTexture(source, streamTextureSourceInfo.compositedTextureHandle, m_textureUploadingAdapter);
        }
    }

    void EmbeddedCompositingManager::destroyStreamTexture(WaylandIviSurfaceId source)
    {
        StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(source);
        assert(streamTextureSourceInfo != nullptr);
        m_device.deleteTexture(streamTextureSourceInfo->compositedTextureHandle);
        m_streamTextureSourceInfoMap.remove(source);
    }
}
