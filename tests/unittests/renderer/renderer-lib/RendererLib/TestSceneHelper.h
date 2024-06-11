//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/Core/Math3d/ProjectionParams.h"
#include "SceneAllocateHelper.h"
#include "RendererResourceManagerMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include <unordered_set>

namespace ramses::internal
{
    class TestSceneHelper
    {
    public:
        explicit TestSceneHelper(IScene& scene, bool indexArrayAvailable = true, bool createDefaultLayouts = true);

        RenderGroupHandle createRenderGroup(RenderPassHandle pass1 = RenderPassHandle::Invalid(), RenderPassHandle pass2 = RenderPassHandle::Invalid());
        RenderableHandle createRenderable(RenderGroupHandle group1 = RenderGroupHandle::Invalid(), RenderGroupHandle group2 = RenderGroupHandle::Invalid(),
            const std::unordered_set<EFixedSemantics>& semantics = {}, RenderableHandle handle = {});
        void removeRenderable(RenderableHandle renderable, RenderGroupHandle group1 = RenderGroupHandle::Invalid(), RenderGroupHandle group2 = RenderGroupHandle::Invalid());
        CameraHandle createCamera(ECameraProjectionType projectionType = ECameraProjectionType::Perspective,
            vec2f frustumNearFar = { 0.1f, 1.f },
            vec4f frustumPlanes = { -1.f, 1.f, -1.f, 1.f },
            vec2f viewportOffset = {},
            vec2f viewportSize = {},
            CameraHandle handle = {});
        CameraHandle createCamera(const ProjectionParams& params, vec2f viewportOffset = {}, vec2f viewportSize = {}, CameraHandle handle = {});
        RenderPassHandle createRenderPassWithCamera();
        BlitPassHandle createBlitPassWithDummyRenderBuffers();
        void createRenderTarget();

        template <typename TextureContentHandle>
        TextureSamplerHandle createTextureSampler(TextureContentHandle handleOrHash);

        TextureSamplerHandle createTextureSamplerWithFakeTexture();
        DataInstanceHandle createAndAssignUniformDataInstance(RenderableHandle renderable, TextureSamplerHandle sampler);
        DataInstanceHandle createAndAssignVertexDataInstance(RenderableHandle renderable);

        void setResourcesToRenderable(RenderableHandle renderable, bool setVertices = true, bool setIndices = true);

        template <typename TextureContentHandle>
        void recreateSamplerWithDifferentContent(TextureSamplerHandle handle, TextureContentHandle contentHandleOrHash);

    public:
        IScene& m_scene;
        SceneAllocateHelper m_sceneAllocator;
        const SceneId m_sceneID;
        StrictMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
        StrictMock<RendererResourceManagerMock>    resourceManager;

        const DataLayoutHandle testUniformLayout            { 0u };
        const DataLayoutHandle testGeometryLayout           { 2u };
        const DataFieldHandle indicesField                  { 0u };
        const DataFieldHandle vertAttribField               { 1u };
        const DataFieldHandle dataField                     { 0u };
        const DataFieldHandle samplerField                  { 1u };
        const RenderTargetHandle renderTarget               { 13u };
        const RenderBufferHandle renderTargetColorBuffer    { 5u };

    private:
        bool m_indexArrayAvailable;
    };
}
