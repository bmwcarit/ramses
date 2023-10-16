//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/Types.h"
#include <mutex>

namespace ramses::internal
{
    class IBinaryShaderCache
    {
    public:
        virtual ~IBinaryShaderCache() = default;

        virtual void deviceSupportsBinaryShaderFormats(const std::vector<BinaryShaderFormatID>& supportedFormats) = 0;

        [[nodiscard]] virtual bool hasBinaryShader(ResourceContentHash effectHash) const = 0;
        [[nodiscard]] virtual uint32_t getBinaryShaderSize(ResourceContentHash effectHash) const = 0;
        [[nodiscard]] virtual BinaryShaderFormatID getBinaryShaderFormat(ResourceContentHash effectHash) const = 0;
        virtual void getBinaryShaderData(ResourceContentHash effectHash, std::byte* buffer, uint32_t bufferSize) const = 0;

        [[nodiscard]] virtual bool shouldBinaryShaderBeCached(ResourceContentHash effectHash, SceneId sceneId) const = 0;

        virtual void storeBinaryShader(ResourceContentHash effectHash, SceneId sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) = 0;
        virtual void binaryShaderUploaded(ResourceContentHash effectHash, bool success) const = 0;
        virtual std::once_flag& binaryShaderFormatsReported() = 0;
    };
}
