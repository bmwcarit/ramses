//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEUPLOADER_H
#define RAMSES_RESOURCEUPLOADER_H

#include "IResourceUploader.h"
#include "Components/ManagedResource.h"
#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class IBinaryShaderCache;
    class TextureResource;
    class EffectResource;
    class IDevice;
    class RendererStatistics;
    struct ResourceDescriptor;

    class ResourceUploader : public IResourceUploader
    {
    public:
        explicit ResourceUploader(RendererStatistics& stats, IBinaryShaderCache* binaryShaderCache = nullptr);

        virtual DeviceResourceHandle uploadResource(IRenderBackend& renderBackend, const ResourceDescriptor& resourceObject, UInt32& outVRAMSize) override;
        virtual void                 unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash hash, DeviceResourceHandle handle) override;

    private:
        DeviceResourceHandle uploadTexture(IDevice& device, const TextureResource& texture, UInt32& vramSize);
        DeviceResourceHandle queryBinaryShaderCacheAndUploadEffect(IRenderBackend& renderBackend, const EffectResource& effect, ResourceContentHash hash, SceneId sceneid);

        static UInt32 EstimateGPUAllocatedSizeOfTexture(const TextureResource& texture, UInt32 numMipLevelsToAllocate);

        IBinaryShaderCache* const m_binaryShaderCache;
        bool m_supportedFormatsReported = false;
        RendererStatistics& m_stats;
    };
}

#endif
