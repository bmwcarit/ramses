//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENESIZEINFORMATION_H
#define RAMSES_SCENESIZEINFORMATION_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    struct SceneSizeInformation
    {
        SceneSizeInformation(
            UInt32 nodes,
            UInt32 cameras,
            UInt32 transforms,
            UInt32 renderables,
            UInt32 states,
            UInt32 datalayouts,
            UInt32 datainstances,
            UInt32 renderGroups,
            UInt32 renderPasses,
            UInt32 blitPasses,
            UInt32 renderTargets,
            UInt32 renderBuffers,
            UInt32 textureSamplers,
            UInt32 streamTextures,
            UInt32 dataSlots,
            UInt32 dataBuffers,
            UInt32 textureBuffers,
            UInt32 pickableObjects)
            : nodeCount(nodes)
            , cameraCount(cameras)
            , transformCount(transforms)
            , renderableCount(renderables)
            , renderStateCount(states)
            , datalayoutCount(datalayouts)
            , datainstanceCount(datainstances)
            , renderGroupCount(renderGroups)
            , renderPassCount(renderPasses)
            , blitPassCount(blitPasses)
            , renderTargetCount(renderTargets)
            , renderBufferCount(renderBuffers)
            , textureSamplerCount(textureSamplers)
            , streamTextureCount(streamTextures)
            , dataSlotCount(dataSlots)
            , dataBufferCount(dataBuffers)
            , textureBufferCount(textureBuffers)
            , pickableObjectCount(pickableObjects)
        {
        }

        SceneSizeInformation()
        {
        }

        bool operator==(const SceneSizeInformation& other) const
        {
            return (nodeCount == other.nodeCount)
                && (cameraCount == other.cameraCount)
                && (transformCount == other.transformCount)
                && (renderableCount == other.renderableCount)
                && (renderStateCount == other.renderStateCount)
                && (datalayoutCount == other.datalayoutCount)
                && (datainstanceCount == other.datainstanceCount)
                && (renderGroupCount == other.renderGroupCount)
                && (renderPassCount == other.renderPassCount)
                && (blitPassCount == other.blitPassCount)
                && (pickableObjectCount == other.pickableObjectCount)
                && (renderTargetCount == other.renderTargetCount)
                && (renderBufferCount == other.renderBufferCount)
                && (textureSamplerCount == other.textureSamplerCount)
                && (streamTextureCount == other.streamTextureCount)
                && (dataSlotCount == other.dataSlotCount)
                && (dataBufferCount == other.dataBufferCount)
                && (textureBufferCount == other.textureBufferCount);
        }

        bool operator>(const SceneSizeInformation& other) const
        {
            return (nodeCount > other.nodeCount)
                || (cameraCount > other.cameraCount)
                || (transformCount > other.transformCount)
                || (renderableCount > other.renderableCount)
                || (renderStateCount > other.renderStateCount)
                || (datalayoutCount > other.datalayoutCount)
                || (datainstanceCount > other.datainstanceCount)
                || (renderGroupCount > other.renderGroupCount)
                || (renderPassCount > other.renderPassCount)
                || (blitPassCount > other.blitPassCount)
                || (pickableObjectCount > other.pickableObjectCount)
                || (renderTargetCount > other.renderTargetCount)
                || (renderBufferCount > other.renderBufferCount)
                || (textureSamplerCount > other.textureSamplerCount)
                || (streamTextureCount > other.streamTextureCount)
                || (dataSlotCount > other.dataSlotCount)
                || (dataBufferCount > other.dataBufferCount)
                || (textureBufferCount > other.textureBufferCount);
        }

        String asString() const
        {
            StringOutputStream str;
            str << "[";
            str << " node=" << nodeCount;
            str << " camera=" << cameraCount;
            str << " transform=" << transformCount;
            str << " renderable=" << renderableCount;
            str << " state=" << renderStateCount;
            str << " datalayout=" << datalayoutCount;
            str << " datainstance=" << datainstanceCount;
            str << " renderGroup=" << renderGroupCount;
            str << " renderPass=" << renderPassCount;
            str << " blitPass=" << blitPassCount;
            str << " pickableObject=" << pickableObjectCount;
            str << " renderTarget=" << renderTargetCount;
            str << " renderBuffer=" << renderBufferCount;
            str << " textureSampler=" << textureSamplerCount;
            str << " streamTexture=" << streamTextureCount;
            str << " dataSlot=" << dataSlotCount;
            str << " dataBuffer=" << dataBufferCount;
            str << " textureBuffer=" << textureBufferCount;
            str << " ]";

            return str.c_str();
        }

        UInt32 nodeCount            = 0u;
        UInt32 cameraCount          = 0u;
        UInt32 transformCount       = 0u;
        UInt32 renderableCount      = 0u;
        UInt32 renderStateCount     = 0u;
        UInt32 datalayoutCount      = 0u;
        UInt32 datainstanceCount    = 0u;
        UInt32 renderGroupCount     = 0u;
        UInt32 renderPassCount      = 0u;
        UInt32 blitPassCount        = 0u;
        UInt32 renderTargetCount    = 0u;
        UInt32 renderBufferCount    = 0u;
        UInt32 textureSamplerCount  = 0u;
        UInt32 streamTextureCount   = 0u;
        UInt32 dataSlotCount        = 0u;
        UInt32 dataBufferCount      = 0u;
        UInt32 textureBufferCount   = 0u;
        UInt32 pickableObjectCount =  0u;
    };
}

#endif
