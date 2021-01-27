//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ASYNCEFFECTUPLOADER_H
#define RAMSES_ASYNCEFFECTUPLOADER_H

#include "Platform_Base/GpuResource.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "SceneAPI/ResourceContentHash.h"

#include <unordered_map>
#include <future>
#include <mutex>
#include <condition_variable>

namespace ramses_internal
{
    class IPlatform;
    class IRenderBackend;
    class IResourceUploadRenderBackend;
    class EffectResource;

    using EffectsGpuResources = std::vector<std::pair<ResourceContentHash, std::unique_ptr<const GPUResource>>>;
    using EffectsRawResources = std::vector<const EffectResource*>;

    class AsyncEffectUploader : private Runnable
    {
    public:
        explicit AsyncEffectUploader(IPlatform& platform, IRenderBackend& renderBackend);
        ~AsyncEffectUploader();

        bool createResourceUploadRenderBackendAndStartThread();
        void destroyResourceUploadRenderBackendAndStopThread();

        void sync(const EffectsRawResources& effectsToUpload, EffectsGpuResources& uploadedResourcesOut);

    private:
        virtual void run() override;
        void uploadEffectsOrWait(IResourceUploadRenderBackend& resourceUploadRenderBackend);

        IPlatform& m_platform;
        IRenderBackend& m_renderBackend;
        PlatformThread m_thread;

        mutable std::mutex m_mutex;
        std::condition_variable m_sleepConditionVar;

        EffectsRawResources m_effectsToUpload;
        EffectsGpuResources m_effectsUploaded;

        EffectsGpuResources m_effectsUploadedCache; //to avoid acquiring mutex twice in resource upload thread

        std::promise<bool> m_creationSuccess;
    };
}

#endif
