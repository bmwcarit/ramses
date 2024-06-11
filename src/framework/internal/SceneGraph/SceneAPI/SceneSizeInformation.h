//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/FmtBase.h"

#include <cstdint>

namespace ramses::internal
{
    struct SceneSizeInformation
    {
        SceneSizeInformation(
            uint32_t nodes,
            uint32_t cameras,
            uint32_t transforms,
            uint32_t renderables,
            uint32_t states,
            uint32_t datalayouts,
            uint32_t datainstances,
            uint32_t uniformBuffers,
            uint32_t renderGroups,
            uint32_t renderPasses,
            uint32_t blitPasses,
            uint32_t renderTargets,
            uint32_t renderBuffers,
            uint32_t textureSamplers,
            uint32_t dataSlots,
            uint32_t dataBuffers,
            uint32_t textureBuffers,
            uint32_t pickableObjects,
            uint32_t sceneReferences)
            : nodeCount(nodes)
            , cameraCount(cameras)
            , transformCount(transforms)
            , renderableCount(renderables)
            , renderStateCount(states)
            , datalayoutCount(datalayouts)
            , datainstanceCount(datainstances)
            , uniformBufferCount(uniformBuffers)
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
                && (uniformBufferCount == other.uniformBufferCount)
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
                || (uniformBufferCount > other.uniformBufferCount)
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

        uint32_t nodeCount            = 0u;
        uint32_t cameraCount          = 0u;
        uint32_t transformCount       = 0u;
        uint32_t renderableCount      = 0u;
        uint32_t renderStateCount     = 0u;
        uint32_t datalayoutCount      = 0u;
        uint32_t datainstanceCount    = 0u;
        uint32_t uniformBufferCount   = 0u;
        uint32_t renderGroupCount     = 0u;
        uint32_t renderPassCount      = 0u;
        uint32_t blitPassCount        = 0u;
        uint32_t renderTargetCount    = 0u;
        uint32_t renderBufferCount    = 0u;
        uint32_t textureSamplerCount  = 0u;
        uint32_t dataSlotCount        = 0u;
        uint32_t dataBufferCount      = 0u;
        uint32_t textureBufferCount   = 0u;
        uint32_t pickableObjectCount  = 0u;
        uint32_t sceneReferenceCount  = 0u;
    };
}

template <>
struct fmt::formatter<ramses::internal::SceneSizeInformation> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::SceneSizeInformation& si, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(),
                              "[node={} camera={} transform={} renderable={} state={} datalayout={} datainstance={} uniformBuffer={} renderGroup={} renderPass={} blitPass={} "
                              "renderTarget={} renderBuffer={} textureSampler={} dataSlot={} dataBuffer={} textureBuffer={} "
                              "pickableObjectCount={} sceneReferenceCount={}]",
                              si.nodeCount,
                              si.cameraCount,
                              si.transformCount,
                              si.renderableCount,
                              si.renderStateCount,
                              si.datalayoutCount,
                              si.datainstanceCount,
                              si.uniformBufferCount,
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

