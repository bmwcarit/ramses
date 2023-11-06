//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/GpuResource.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"

#include <unordered_map>
#include <future>
#include <mutex>
#include <condition_variable>

namespace ramses::internal
{
    class IPlatform;
    class IRenderBackend;
    class IResourceUploadRenderBackend;
    class EffectResource;
    class IThreadAliveNotifier;

    using EffectsGpuResources = std::vector<std::pair<ResourceContentHash, std::unique_ptr<const GPUResource>>>;
    using EffectsRawResources = std::vector<const EffectResource*>;

    class AsyncEffectUploader : private Runnable
    {
    public:
        AsyncEffectUploader(IPlatform& platform, IRenderBackend& renderBackend, IThreadAliveNotifier& notifier, DisplayHandle display);
        ~AsyncEffectUploader() override;

        bool createResourceUploadRenderBackendAndStartThread();
        void destroyResourceUploadRenderBackendAndStopThread();

        void sync(const EffectsRawResources& effectsToUpload, EffectsGpuResources& uploadedResourcesOut);

    private:
        void run() override;
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

        IThreadAliveNotifier& m_notifier;
        const uint64_t m_aliveIdentifier;

        const DisplayHandle m_displayHandle;
    };
}
