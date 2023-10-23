//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/TextureSamplerStates.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

namespace ramses::internal
{
    struct TextureSampler
    {
        enum class ContentType : uint8_t
        {
            None = 0,
            ClientTexture,
            TextureBuffer,
            RenderBuffer,
            RenderBufferMS,
            OffscreenBuffer,
            StreamBuffer,
            ExternalTexture
        };

        TextureSampler() = default;

        TextureSampler(const TextureSamplerStates& states_, ContentType type, const ResourceContentHash& texHash, MemoryHandle handle)
            : states(states_)
            , contentType(type)
            , textureResource(texHash)
            , contentHandle(handle)
        {
        }

        TextureSampler(const TextureSamplerStates& states_, const ResourceContentHash& texHash)
            : TextureSampler(states_, ContentType::ClientTexture, texHash, InvalidMemoryHandle)
        {
        }

        TextureSampler(const TextureSamplerStates& states_, TextureBufferHandle handle)
            : TextureSampler(states_, ContentType::TextureBuffer, {}, handle.asMemoryHandle())
        {
        }

        TextureSampler(const TextureSamplerStates& states_, RenderBufferHandle handle)
            : TextureSampler(states_, ContentType::RenderBuffer, {}, handle.asMemoryHandle())
        {
        }

        TextureSamplerStates states;
        ContentType          contentType = ContentType::None;
        ResourceContentHash  textureResource;
        MemoryHandle         contentHandle = InvalidMemoryHandle;

        [[nodiscard]] bool isRenderBuffer() const
        {
            switch (contentType)
            {
            case ramses::internal::TextureSampler::ContentType::RenderBuffer:
            case ramses::internal::TextureSampler::ContentType::RenderBufferMS:
                return true;
            case ramses::internal::TextureSampler::ContentType::None:
            case ramses::internal::TextureSampler::ContentType::ClientTexture:
            case ramses::internal::TextureSampler::ContentType::TextureBuffer:
            case ramses::internal::TextureSampler::ContentType::OffscreenBuffer:
            case ramses::internal::TextureSampler::ContentType::StreamBuffer:
            case ramses::internal::TextureSampler::ContentType::ExternalTexture:
                break;
            }
            return false;
        }
    };
}
