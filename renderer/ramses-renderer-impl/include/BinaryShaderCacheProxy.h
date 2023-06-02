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
        ~BinaryShaderCacheProxy() override = default;

        void deviceSupportsBinaryShaderFormats(const std::vector<ramses_internal::BinaryShaderFormatID>& supportedFormats) override;

        bool hasBinaryShader(ramses_internal::ResourceContentHash effectHash) const override;
        uint32_t getBinaryShaderSize(ramses_internal::ResourceContentHash effectHash) const override;
        ramses_internal::BinaryShaderFormatID getBinaryShaderFormat(ramses_internal::ResourceContentHash effectHash) const override;
        void getBinaryShaderData(ramses_internal::ResourceContentHash effectHash, uint8_t* buffer, ramses_internal::UInt32 bufferSize) const override;

        bool shouldBinaryShaderBeCached(ramses_internal::ResourceContentHash effectHash, ramses_internal::SceneId sceneId) const override;
        void storeBinaryShader(ramses_internal::ResourceContentHash effectHash, ramses_internal::SceneId sceneId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, ramses_internal::BinaryShaderFormatID binaryShaderFormat) override;
        void binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const override;
        std::once_flag& binaryShaderFormatsReported() override;

    private:
        ramses::IBinaryShaderCache& m_cache;
        mutable std::mutex          m_mutex;
        std::once_flag              m_supportedFormatsReported;
    };
}

#endif
