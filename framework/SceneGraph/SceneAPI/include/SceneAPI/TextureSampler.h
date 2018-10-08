//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_TEXTURESAMPLER_H
#define RAMSES_SCENEAPI_TEXTURESAMPLER_H

#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/TextureSamplerStates.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    struct TextureSampler
    {
        enum class ContentType : UInt8
        {
            None = 0,
            ClientTexture,
            TextureBuffer,
            RenderBuffer,
            StreamTexture,
            OffscreenBuffer
        };

        TextureSampler()
        {
        }

        TextureSampler(const TextureSamplerStates& states_, ContentType type, const ResourceContentHash& texHash, MemoryHandle handle)
            : states(states_)
            , contentType(type)
            , textureResource(texHash)
            , contentHandle(handle)
        {
        }

        TextureSampler(const TextureSamplerStates& states_, const ResourceContentHash& texHash)
            : states(states_)
            , contentType(ContentType::ClientTexture)
            , textureResource(texHash)
        {
        }

        TextureSampler(const TextureSamplerStates& states_, TextureBufferHandle handle)
            : states(states_)
            , contentType(ContentType::TextureBuffer)
            , contentHandle(handle.asMemoryHandle())
        {
        }

        TextureSampler(const TextureSamplerStates& states_, RenderBufferHandle handle)
            : states(states_)
            , contentType(ContentType::RenderBuffer)
            , contentHandle(handle.asMemoryHandle())
        {
        }

        TextureSampler(const TextureSamplerStates& states_, StreamTextureHandle handle)
            : states(states_)
            , contentType(ContentType::StreamTexture)
            , contentHandle(handle.asMemoryHandle())
        {
        }

        TextureSamplerStates states;
        ContentType          contentType = ContentType::None;
        ResourceContentHash  textureResource;
        MemoryHandle         contentHandle = InvalidMemoryHandle;
    };
}

#endif
