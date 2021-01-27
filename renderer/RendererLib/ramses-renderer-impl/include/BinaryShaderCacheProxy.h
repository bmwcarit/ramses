//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYSHADERCACHEPROXY_H
#define RAMSES_BINARYSHADERCACHEPROXY_H

#include "RendererAPI/IBinaryShaderCache.h"
#include "ramses-renderer-api/Types.h"
#include <mutex>

namespace ramses
{
    class IBinaryShaderCache;
    class BinaryShaderCacheProxy final : public ramses_internal::IBinaryShaderCache
    {
    public:
        explicit BinaryShaderCacheProxy(ramses::IBinaryShaderCache& cache);
        virtual ~BinaryShaderCacheProxy() = default;

        virtual void deviceSupportsBinaryShaderFormats(const std::vector<ramses_internal::BinaryShaderFormatID>& supportedFormats) override;

        virtual bool hasBinaryShader(ramses_internal::ResourceContentHash effectHash) const override;
        virtual uint32_t getBinaryShaderSize(ramses_internal::ResourceContentHash effectHash) const override;
        virtual ramses_internal::BinaryShaderFormatID getBinaryShaderFormat(ramses_internal::ResourceContentHash effectHash) const override;
        virtual void getBinaryShaderData(ramses_internal::ResourceContentHash effectHash, ramses_internal::UInt8* buffer, ramses_internal::UInt32 bufferSize) const override;

        virtual bool shouldBinaryShaderBeCached(ramses_internal::ResourceContentHash effectHash, ramses_internal::SceneId sceneId) const override;
        virtual void storeBinaryShader(ramses_internal::ResourceContentHash effectHash, ramses_internal::SceneId sceneId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, ramses_internal::BinaryShaderFormatID binaryShaderFormat) override;
        virtual void binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const override;
        virtual std::once_flag& binaryShaderFormatsReported() override;

    private:
        ramses::IBinaryShaderCache& m_cache;
        mutable std::mutex          m_mutex;
        std::once_flag              m_supportedFormatsReported;
    };
}

#endif
