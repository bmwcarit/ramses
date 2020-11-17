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

        virtual void refStream(StreamTextureHandle handle, WaylandIviSurfaceId source, SceneId sceneId) override;
        virtual void unrefStream(StreamTextureHandle handle, WaylandIviSurfaceId source, SceneId sceneId) override;
        virtual void refStream(WaylandIviSurfaceId source) override;
        virtual void unrefStream(WaylandIviSurfaceId source) override;

        virtual void dispatchStateChangesOfStreamTexturesAndSources(SceneStreamTextures& streamTexturesWithStateChange, WaylandIviSurfaceIdVector& newStreams, WaylandIviSurfaceIdVector& obsoleteStreams) override;
        virtual void processClientRequests() override;
        virtual Bool hasUpdatedContentFromStreamSourcesToUpload() const override;
        virtual void uploadResourcesAndGetUpdates(UpdatedSceneIdSet &updatedScenes, StreamTextureBufferUpdates& bufferUpdates) override;
        virtual void notifyClients() override;
        virtual DeviceResourceHandle getCompositedTextureDeviceHandleForStreamTexture(WaylandIviSurfaceId source) const override;
        virtual Bool hasRealCompositor() const override; //TODO Mohamed: remove this as soon as EC dummy is removed

    private:
        struct StreamSourceSceneUsageEntry
        {
            SceneId sceneId;
            StreamTextureHandle streamTextureHandle;
        };
        using StreamSourceSceneUsageEntryVector = std::vector<StreamSourceSceneUsageEntry>;

        struct StreamTextureSourceInfo
        {
            DeviceResourceHandle compositedTextureHandle;
            StreamSourceSceneUsageEntryVector sceneUsage;
            int streamBufferUsage = 0;
            bool contentAvailable = false;
        };

        void createStreamTexture(WaylandIviSurfaceId source);
        void destroyStreamTexture(WaylandIviSurfaceId source);
        static void AddStreamTexturesWithStateChange(SceneStreamTextures& result, const StreamSourceSceneUsageEntryVector& streamTexturesWithStateChange);

        IDevice& m_device;
        IEmbeddedCompositor& m_embeddedCompositor;
        HashMap<WaylandIviSurfaceId, StreamTextureSourceInfo> m_streamTextureSourceInfoMap;
        ITextureUploadingAdapter& m_textureUploadingAdapter;
    };
}

#endif
