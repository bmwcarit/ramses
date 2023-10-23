//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/IResourceUploader.h"
#include "internal/Components/ManagedResource.h"

namespace ramses::internal
{
    class IBinaryShaderCache;
    class TextureResource;
    class EffectResource;
    class IDevice;
    struct ResourceDescriptor;

    class ResourceUploader : public IResourceUploader
    {
    public:
        explicit ResourceUploader(bool asyncEffectUploadEnabled, IBinaryShaderCache* binaryShaderCache = nullptr);

        std::optional<DeviceResourceHandle> uploadResource(IRenderBackend& renderBackend, const ResourceDescriptor& rd, uint32_t& outVRAMSize) override;
        void                 unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash hash, DeviceResourceHandle handle) override;
        void                         storeShaderInBinaryShaderCache(IRenderBackend& renderBackend, DeviceResourceHandle deviceHandle, const ResourceContentHash& hash, SceneId sceneid) override;

    private:
        static DeviceResourceHandle UploadTexture(IDevice& device, const TextureResource& texture, uint32_t& vramSize);
        DeviceResourceHandle queryBinaryShaderCache(IRenderBackend& renderBackend, const EffectResource& effect, ResourceContentHash hash);

        static uint32_t EstimateGPUAllocatedSizeOfTexture(const TextureResource& texture, uint32_t numMipLevelsToAllocate);

        const bool m_asyncEffectUploadEnabled;
        IBinaryShaderCache* const m_binaryShaderCache;
    };
}
