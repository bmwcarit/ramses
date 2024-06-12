//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "Triangle.h"
#include "Line.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "ramses/client/MeshNode.h"
#include "MultiTriangleGeometry.h"

namespace ramses::internal
{
    class MultipleTrianglesScene : public IntegrationScene
    {
    public:
        MultipleTrianglesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        void setState(uint32_t state);

        enum
        {
            THREE_TRIANGLES = 0,
            TRIANGLES_REORDERED,
            ADDITIVE_BLENDING,
            SUBTRACTIVE_BLENDING,
            ALPHA_BLENDING,
            BLENDING_CONSTANT,
            BLENDING_DST_COLOR_AND_ALPHA,
            COLOR_MASK,
            CAMERA_TRANSFORMATION,
            CAMERA_TRANSFORMATION_UBO,
            FACE_CULLING,
            DEPTH_FUNC,
            DRAW_MODE,
            STENCIL_TEST_1,
            STENCIL_TEST_2,
            STENCIL_TEST_3,
            SCISSOR_TEST,
            MULTIPLE_DISPLAYS,
            SUBIMAGES,
            PERSPECTIVE_CAMERA,
            ORTHOGRAPHIC_CAMERA,
            THREE_TRIANGLES_WITH_SHARED_COLOR,
            THREE_TRIANGLES_WITH_UNSHARED_COLOR,
            EULER_ROTATION_CONVENTIONS,
            PERSPECTIVE_CAMERA_UBO,
            ORTHOGRAPHIC_CAMERA_UBO,
        };
    private:
        Effect* createTestEffect(uint32_t state);
        void setGeometries(uint32_t state);
        void setTransformations(uint32_t state);

        ramses::Effect  * m_Effect;

        ramses::MeshNode* m_meshNode1;
        ramses::MeshNode* m_meshNode2;
        ramses::MeshNode* m_meshNode3;
        ramses::MeshNode* m_meshNode4;
        ramses::MeshNode* m_meshNode5;
        ramses::MeshNode* m_meshNode6;
        ramses::MeshNode* m_meshNode7;

        Triangle m_whiteTriangle;
        Triangle m_redTriangle;
        Triangle m_greenTriangle;
        Triangle m_blueTriangle;
        Line     m_yellowLine;
        MultiTriangleGeometry m_whiteQuad;
        MultiTriangleGeometry m_triangleFan;
        Line     m_lineStrip;
        Line     m_linePoints;
        Triangle m_redTransparentTriangle;
        Triangle m_greenTransparentTriangle;
        Triangle m_blueTransparentTriangle;
        Triangle m_colorMaskRedTriangle;
        Triangle m_colorMaskGreenTriangle;
        Triangle m_colorMaskBlueTriangle;
        Triangle m_CCWTriangle;
        Triangle m_CWTriangle;
        Triangle m_CWTriangleCCWIndices;
    };
}
