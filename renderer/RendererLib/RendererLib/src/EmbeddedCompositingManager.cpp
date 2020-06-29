//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/EmbeddedCompositingManager.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/ITextureUploadingAdapter.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    EmbeddedCompositingManager::EmbeddedCompositingManager(IDevice &device, IEmbeddedCompositor &embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter)
        : m_device(device)
        , m_embeddedCompositor(embeddedCompositor)
        , m_textureUploadingAdapter(textureUploadingAdapter)
    {
    }

    void EmbeddedCompositingManager::uploadStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId)
    {
        auto streamTextureInfoIt = m_streamTextureSourceInfoMap.find(source);
        if(m_streamTextureSourceInfoMap.end() == streamTextureInfoIt)
        {
            //composited stream texture exists
            LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositingManager::uploadStreamTexture New stream texture with source id :" << source.getValue());
            uploadNewStreamTexture(source);

            streamTextureInfoIt = m_streamTextureSourceInfoMap.find(source);
        }

        LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositingManager::uploadStreamTexture adding scene usage for stream texture with source id :" << source.getValue());
        const StreamSourceSceneUsageEntry streamSourceUsageInScene = {sceneId, handle};
        streamTextureInfoIt->value.sceneUsage.push_back(streamSourceUsageInScene);
    }

    void EmbeddedCompositingManager::deleteStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId)
    {
        StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(source);
        assert(nullptr != streamTextureSourceInfo);

        //remove scene usage instance
        auto& sceneUsage = streamTextureSourceInfo->sceneUsage;
        for(auto scene = sceneUsage.begin(); scene != sceneUsage.end(); ++scene)
        {
            if (scene->sceneId == sceneId && scene->streamTextureHandle == handle)
            {
                sceneUsage.erase(scene);
                break;
            }
        }

        if(sceneUsage.empty())
        {
            //last reference for for stream texture source
            //remove composited texture from GPU and remove entry from hashmap
            m_device.deleteTexture(streamTextureSourceInfo->compositedTextureHandle);
            m_streamTextureSourceInfoMap.remove(source);
        }
    }

    void EmbeddedCompositingManager::dispatchStateChangesOfStreamTexturesAndSources(SceneStreamTextures& streamTexturesWithStateChange, StreamTextureSourceIdVector& newStreams, StreamTextureSourceIdVector& obsoleteStreams)
    {
        assert(0u == streamTexturesWithStateChange.size());
        assert(0u == newStreams.size());
        assert(0u == obsoleteStreams.size());

        const auto newStreamsSet = m_embeddedCompositor.dispatchNewStreamTextureSourceIds();
        const auto obsoleteStreamsSet = m_embeddedCompositor.dispatchObsoleteStreamTextureSourceIds();

        for(auto s : newStreamsSet)
            newStreams.push_back(s);
        for(auto s : obsoleteStreamsSet)
            obsoleteStreams.push_back(s);

        for(auto& streamTexture : m_streamTextureSourceInfoMap)
        {
            const Bool contentAvailable = m_embeddedCompositor.isContentAvailableForStreamTexture(streamTexture.key);
            StreamTextureSourceInfo& streamSourceInfo = streamTexture.value;
            if (contentAvailable != streamSourceInfo.contentAvailable)
            {
                AddStreamTexturesWithStateChange(streamTexturesWithStateChange, streamSourceInfo.sceneUsage);
            }

            streamSourceInfo.contentAvailable = contentAvailable;
        }
    }

    DeviceResourceHandle EmbeddedCompositingManager::getCompositedTextureDeviceHandleForStreamTexture(StreamTextureSourceId source) const
    {
        if(m_embeddedCompositor.isContentAvailableForStreamTexture(source))
        {
            StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(source);
            assert(nullptr != streamTextureSourceInfo);
            return streamTextureSourceInfo->compositedTextureHandle;
        }

        return DeviceResourceHandle::Invalid();
    }

    Bool EmbeddedCompositingManager::hasRealCompositor() const
    {
        return m_embeddedCompositor.isRealCompositor();
    }

    void EmbeddedCompositingManager::AddStreamTexturesWithStateChange(SceneStreamTextures& result, const StreamSourceSceneUsageEntryVector& streamTexturesWithStateChange)
    {
        for(const auto& streamTexture : streamTexturesWithStateChange)
        {
            if (!result.contains(streamTexture.sceneId))
            {
                result.put(streamTexture.sceneId, StreamTextureHandleVector());
            }

            StreamTextureHandleVector* sceneStreamTextures = result.get(streamTexture.sceneId);
            sceneStreamTextures->push_back(streamTexture.streamTextureHandle);
        }
    }

    void EmbeddedCompositingManager::processClientRequests()
    {
        m_embeddedCompositor.handleRequestsFromClients();
        m_embeddedCompositor.endFrame(false);
    }

    Bool EmbeddedCompositingManager::hasUpdatedContentFromStreamSourcesToUpload() const
    {
        return m_embeddedCompositor.hasUpdatedStreamTextureSources();
    }

    void EmbeddedCompositingManager::uploadResourcesAndGetUpdates(UpdatedSceneIdSet &updatedScenes, StreamTextureBufferUpdates& bufferUpdates)
    {
        const StreamTextureSourceIdSet updatedStreamTextureSourceIds = m_embeddedCompositor.dispatchUpdatedStreamTextureSourceIds();
        for(const auto streamTextureSourceId : updatedStreamTextureSourceIds)
        {
            const StreamTextureSourceInfo* streamTextureSourceInfo = m_streamTextureSourceInfoMap.get(streamTextureSourceId);
            if(nullptr != streamTextureSourceInfo)
            {
                const UInt32 bufferUpdateCount = m_embeddedCompositor.uploadCompositingContentForStreamTexture(streamTextureSourceId, streamTextureSourceInfo->compositedTextureHandle, m_textureUploadingAdapter);
                bufferUpdates[streamTextureSourceId] = bufferUpdateCount;

                for(const auto& it : streamTextureSourceInfo->sceneUsage)
                {
                    updatedScenes.put(it.sceneId);
                }
            }
        }
    }

    void EmbeddedCompositingManager::notifyClients()
    {
        m_embeddedCompositor.endFrame(true);
    }

    void EmbeddedCompositingManager::uploadNewStreamTexture(StreamTextureSourceId source)
    {
        // register a new texture (with dummy description and no data) to device and put entry in hashmap
        // actual data is uploaded later when source content available
        const DeviceResourceHandle compositedTextureDeviceHandle = m_device.uploadStreamTexture2D(DeviceResourceHandle::Invalid(), 1u, 1u, ETextureFormat_RGBA8, nullptr, {ETextureChannelColor::Blue, ETextureChannelColor::Green, ETextureChannelColor::Red, ETextureChannelColor::Alpha});

        StreamTextureSourceInfo streamTextureSourceInfo;
        streamTextureSourceInfo.compositedTextureHandle = compositedTextureDeviceHandle;
        streamTextureSourceInfo.contentAvailable = false;
        m_streamTextureSourceInfoMap.put(source, streamTextureSourceInfo);

        if (m_embeddedCompositor.isContentAvailableForStreamTexture(source))
        {
            LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositingManager::uploadStreamTexture Content available for stream texture " << source.getValue());
            m_embeddedCompositor.uploadCompositingContentForStreamTexture(source, streamTextureSourceInfo.compositedTextureHandle, m_textureUploadingAdapter);
        }
    }
}
