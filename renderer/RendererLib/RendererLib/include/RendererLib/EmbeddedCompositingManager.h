//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITINGMANAGER_H
#define RAMSES_EMBEDDEDCOMPOSITINGMANAGER_H

#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class IDevice;
    class IEmbeddedCompositor;
    class ITextureUploadingAdapter;

    class EmbeddedCompositingManager: public IEmbeddedCompositingManager
    {
    public:
        EmbeddedCompositingManager(IDevice& device, IEmbeddedCompositor& embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter);

        virtual void uploadStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId) override;
        virtual void deleteStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId) override;
        virtual void dispatchStateChangesOfStreamTexturesAndSources(SceneStreamTextures& streamTexturesWithStateChange, StreamTextureSourceIdVector& newStreams, StreamTextureSourceIdVector& obsoleteStreams) override;
        virtual void processClientRequests() override;
        virtual Bool hasUpdatedContentFromStreamSourcesToUpload() const override;
        virtual void uploadResourcesAndGetUpdates(UpdatedSceneIdSet &updatedScenes, StreamTextureBufferUpdates& bufferUpdates) override;
        virtual void notifyClients() override;
        virtual DeviceResourceHandle getCompositedTextureDeviceHandleForStreamTexture(StreamTextureSourceId source) const override;
        virtual Bool hasRealCompositor() const override; //TODO Mohamed: remove this as soon as EC dummy is removed

    private:
        struct StreamSourceSceneUsageEntry
        {
            SceneId sceneId;
            StreamTextureHandle streamTextureHandle;
        };
        typedef std::vector<StreamSourceSceneUsageEntry> StreamSourceSceneUsageEntryVector;

        struct StreamTextureSourceInfo
        {
            DeviceResourceHandle compositedTextureHandle;
            StreamSourceSceneUsageEntryVector sceneUsage;
            Bool contentAvailable;
        };

        void uploadNewStreamTexture(StreamTextureSourceId source);
        static void AddStreamTexturesWithStateChange(SceneStreamTextures& result, const StreamSourceSceneUsageEntryVector& streamTexturesWithStateChange);

        IDevice& m_device;
        IEmbeddedCompositor& m_embeddedCompositor;
        HashMap<StreamTextureSourceId, StreamTextureSourceInfo> m_streamTextureSourceInfoMap;
        ITextureUploadingAdapter& m_textureUploadingAdapter;
    };
}

#endif
