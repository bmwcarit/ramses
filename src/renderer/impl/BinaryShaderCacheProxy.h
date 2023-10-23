//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IBinaryShaderCache.h"
#include "ramses/renderer/Types.h"
#include <mutex>

namespace ramses
{
    class IBinaryShaderCache;
}

namespace ramses::internal
{
    class BinaryShaderCacheProxy final : public ramses::internal::IBinaryShaderCache
    {
    public:
        explicit BinaryShaderCacheProxy(ramses::IBinaryShaderCache& cache);
        ~BinaryShaderCacheProxy() override = default;

        void deviceSupportsBinaryShaderFormats(const std::vector<BinaryShaderFormatID>& supportedFormats) override;

        bool hasBinaryShader(ResourceContentHash effectHash) const override;
        uint32_t getBinaryShaderSize(ResourceContentHash effectHash) const override;
        BinaryShaderFormatID getBinaryShaderFormat(ResourceContentHash effectHash) const override;
        void getBinaryShaderData(ResourceContentHash effectHash, std::byte* buffer, uint32_t bufferSize) const override;

        bool shouldBinaryShaderBeCached(ResourceContentHash effectHash, SceneId sceneId) const override;
        void storeBinaryShader(ResourceContentHash effectHash, SceneId sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) override;
        void binaryShaderUploaded(ResourceContentHash effectHash, bool success) const override;
        std::once_flag& binaryShaderFormatsReported() override;

    private:
        ramses::IBinaryShaderCache& m_cache;
        mutable std::mutex          m_mutex;
        std::once_flag              m_supportedFormatsReported;
    };
}
