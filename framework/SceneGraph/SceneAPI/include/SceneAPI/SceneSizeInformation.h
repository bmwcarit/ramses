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
#include "PlatformAbstraction/FmtBase.h"

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
            UInt32 dataSlots,
            UInt32 dataBuffers,
            UInt32 textureBuffers,
            UInt32 pickableObjects,
            UInt32 sceneReferences)
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
            , dataSlotCount(dataSlots)
            , dataBufferCount(dataBuffers)
            , textureBufferCount(textureBuffers)
            , pickableObjectCount(pickableObjects)
            , sceneReferenceCount(sceneReferences)
        {
        }

        SceneSizeInformation() = default;

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
                && (renderTargetCount == other.renderTargetCount)
                && (renderBufferCount == other.renderBufferCount)
                && (textureSamplerCount == other.textureSamplerCount)
                && (dataSlotCount == other.dataSlotCount)
                && (dataBufferCount == other.dataBufferCount)
                && (textureBufferCount == other.textureBufferCount)
                && (pickableObjectCount == other.pickableObjectCount)
                && (sceneReferenceCount == other.sceneReferenceCount);
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
                || (dataSlotCount > other.dataSlotCount)
                || (dataBufferCount > other.dataBufferCount)
                || (textureBufferCount > other.textureBufferCount)
                || (pickableObjectCount > other.pickableObjectCount)
                || (sceneReferenceCount > other.sceneReferenceCount);
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
        UInt32 dataSlotCount        = 0u;
        UInt32 dataBufferCount      = 0u;
        UInt32 textureBufferCount   = 0u;
        UInt32 pickableObjectCount  = 0u;
        UInt32 sceneReferenceCount  = 0u;
    };
}

template <>
struct fmt::formatter<ramses_internal::SceneSizeInformation> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::SceneSizeInformation& si, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(),
                              "[node={} camera={} transform={} renderable={} state={} datalayout={} datainstance={} renderGroup={} renderPass={} blitPass={} "
                              "renderTarget={} renderBuffer={} textureSampler={} dataSlot={} dataBuffer={} textureBuffer={} "
                              "pickableObjectCount={} sceneReferenceCount={}]",
                              si.nodeCount,
                              si.cameraCount,
                              si.transformCount,
                              si.renderableCount,
                              si.renderStateCount,
                              si.datalayoutCount,
                              si.datainstanceCount,
                              si.renderGroupCount,
                              si.renderPassCount,
                              si.blitPassCount,
                              si.renderTargetCount,
                              si.renderBufferCount,
                              si.textureSamplerCount,
                              si.dataSlotCount,
                              si.dataBufferCount,
                              si.textureBufferCount,
                              si.pickableObjectCount,
                              si.sceneReferenceCount);
    }
};


#endif
