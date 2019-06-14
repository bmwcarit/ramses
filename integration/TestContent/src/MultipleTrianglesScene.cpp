//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultipleTrianglesScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/DataVector4f.h"

namespace ramses_internal
{
    MultipleTrianglesScene::MultipleTrianglesScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_Effect(getTestEffect("ramses-test-client-basic"))
        , m_meshNode1(0)
        , m_meshNode2(0)
        , m_meshNode3(0)
        , m_meshNode4(0)
        , m_meshNode5(0)
        , m_redTriangle             (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_Red)
        , m_greenTriangle           (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_Green)
        , m_blueTriangle            (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_Blue)
        , m_yellowLine              (ramsesClient, scene, *m_Effect, ramses::Line::EColor_Yellow)
        , m_whiteQuad               (ramsesClient, scene, *m_Effect, ramses::TriangleStripQuad::EColor_White)
        , m_redTransparentTriangle  (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_Red, 0.6f)
        , m_greenTransparentTriangle(ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_Green, 0.6f)
        , m_blueTransparentTriangle (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_Blue, 0.6f)
        , m_colorMaskRedTriangle    (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_White)
        , m_colorMaskGreenTriangle  (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_White)
        , m_colorMaskBlueTriangle   (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_White)
        , m_CCWTriangle             (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_White, 1.f, ramses::TriangleGeometry::EVerticesOrder_CCW)
        , m_CWTriangle              (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_White, 1.f, ramses::TriangleGeometry::EVerticesOrder_CW)
        , m_CWTriangleCCWIndices    (ramsesClient, scene, *m_Effect, ramses::TriangleAppearance::EColor_White, 1.f, ramses::TriangleGeometry::EVerticesOrder_CW)
    {
        m_colorMaskRedTriangle.GetAppearance().setColorWriteMask(false, true, true, true);
        m_colorMaskGreenTriangle.GetAppearance().setColorWriteMask(true, false, true, true);
        m_colorMaskBlueTriangle.GetAppearance().setColorWriteMask(true, true, false, true);

        // create a mesh node to define the triangle with chosen appearance
        m_meshNode1 = m_scene.createMeshNode("red triangle mesh node");
        m_meshNode2 = m_scene.createMeshNode("green triangle mesh node");
        m_meshNode3 = m_scene.createMeshNode("blue triangle mesh node");
        m_meshNode4 = m_scene.createMeshNode("White quad mesh node");
        m_meshNode5 = m_scene.createMeshNode("Yellow triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*m_meshNode1);
        addMeshNodeToDefaultRenderGroup(*m_meshNode2);
        addMeshNodeToDefaultRenderGroup(*m_meshNode3);

        m_meshNode1->setGeometryBinding(m_redTriangle.GetGeometry());
        m_meshNode2->setGeometryBinding(m_greenTriangle.GetGeometry());
        m_meshNode3->setGeometryBinding(m_blueTriangle.GetGeometry());
        m_meshNode4->setGeometryBinding(m_whiteQuad.GetGeometry());
        m_meshNode5->setGeometryBinding(m_yellowLine.GetGeometry());

        ramses::Node* transNode1 = m_scene.createNode();
        ramses::Node* transNode2 = m_scene.createNode();
        ramses::Node* transNode3 = m_scene.createNode();
        ramses::Node* transNode4 = m_scene.createNode();
        ramses::Node* transNode5 = m_scene.createNode();

        transNode1->setTranslation(0.f, -0.2f, -12.f);
        transNode2->setTranslation(-0.2f, 0.f, -11.f);
        transNode3->setTranslation(0.2f, 0.2f, -10.f);
        transNode4->setTranslation(-0.2f, -0.2f, -9.f);
        transNode5->setTranslation(-0.3f, -0.2f, -8.f);

        m_meshNode1->setParent(*transNode1);
        m_meshNode2->setParent(*transNode2);
        m_meshNode3->setParent(*transNode3);
        m_meshNode4->setParent(*transNode4);
        m_meshNode5->setParent(*transNode5);

        setState(state);
    }

    void MultipleTrianglesScene::setState(UInt32 state)
    {
        ramses::Node* rotate = 0;
        ramses::Node* translate = 0;

        switch (state)
        {
        case SUBIMAGES:
        case MULTIPLE_DISPLAYS:
        case THREE_TRIANGLES:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            break;
        case TRIANGLES_REORDERED:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            m_redTriangle.GetAppearance().setDepthWrite(ramses::EDepthWrite_Disabled);
            m_greenTriangle.GetAppearance().setDepthWrite(ramses::EDepthWrite_Disabled);
            m_blueTriangle.GetAppearance().setDepthWrite(ramses::EDepthWrite_Disabled);
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 99);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, -1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 1);
            break;
        case ADDITIVE_BLENDING:
            m_redTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_One, ramses::EBlendFactor_One, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_redTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            m_greenTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_One, ramses::EBlendFactor_One, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_greenTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            m_blueTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_One, ramses::EBlendFactor_One, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_blueTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            m_meshNode1->setAppearance(m_redTransparentTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTransparentTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTransparentTriangle.GetAppearance());
            m_meshNode1->setGeometryBinding(m_redTransparentTriangle.GetGeometry());
            m_meshNode2->setGeometryBinding(m_greenTransparentTriangle.GetGeometry());
            m_meshNode3->setGeometryBinding(m_blueTransparentTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case SUBTRACTIVE_BLENDING:
            m_greenTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_One, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_greenTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_ReverseSubtract, ramses::EBlendOperation_Add);
            m_blueTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_One, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_blueTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_ReverseSubtract, ramses::EBlendOperation_Add);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTransparentTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTransparentTriangle.GetAppearance());
            m_meshNode1->setGeometryBinding(m_redTriangle.GetGeometry());
            m_meshNode2->setGeometryBinding(m_greenTransparentTriangle.GetGeometry());
            m_meshNode3->setGeometryBinding(m_blueTransparentTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case ALPHA_BLENDING:
            m_redTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_redTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            m_greenTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_greenTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            m_blueTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
            m_blueTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            m_meshNode1->setAppearance(m_redTransparentTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTransparentTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTransparentTriangle.GetAppearance());
            m_meshNode1->setGeometryBinding(m_redTransparentTriangle.GetGeometry());
            m_meshNode2->setGeometryBinding(m_greenTransparentTriangle.GetGeometry());
            m_meshNode3->setGeometryBinding(m_blueTransparentTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case COLOR_MASK:
            m_meshNode1->setAppearance(m_colorMaskRedTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_colorMaskGreenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_colorMaskBlueTriangle.GetAppearance());
            m_meshNode1->setGeometryBinding(m_colorMaskRedTriangle.GetGeometry());
            m_meshNode2->setGeometryBinding(m_colorMaskGreenTriangle.GetGeometry());
            m_meshNode3->setGeometryBinding(m_colorMaskBlueTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 0);
            break;
        case CAMERA_TRANSFORMATION:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());

            rotate = m_scene.createNode();
            translate = m_scene.createNode();

            getDefaultCamera().setParent(*rotate);
            rotate->setParent(*translate);

            translate->setTranslation(-3.f, 3.5f, 0.f);
            rotate->setRotation(2.0f, 22.0f, 60.0f);
            break;
        case FACE_CULLING:
            m_CCWTriangle.GetAppearance().setCullingMode(ramses::ECullMode_BackFacing);
            m_CWTriangle.GetAppearance().setCullingMode(ramses::ECullMode_FrontFacing);
            m_CWTriangleCCWIndices.GetAppearance().setCullingMode(ramses::ECullMode_BackFacing);
            m_meshNode1->setAppearance(m_CCWTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_CWTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_CWTriangleCCWIndices.GetAppearance());
            m_meshNode1->setGeometryBinding(m_CCWTriangle.GetGeometry());
            m_meshNode2->setGeometryBinding(m_CWTriangle.GetGeometry());
            m_meshNode3->setGeometryBinding(m_CWTriangleCCWIndices.GetGeometry());
            break;
        case DEPTH_FUNC:
            m_redTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc_Greater);
            m_greenTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc_Greater);
            m_blueTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc_LessEqual);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 0);
            break;
        case DRAW_MODE:
            m_redTriangle.GetAppearance().setDrawMode(ramses::EDrawMode_Triangles);
            m_yellowLine.GetAppearance().setDrawMode(ramses::EDrawMode_Lines);
            m_greenTriangle.GetAppearance().setDrawMode(ramses::EDrawMode_LineLoop);
            m_blueTriangle.GetAppearance().setDrawMode(ramses::EDrawMode_Points);
            addMeshNodeToDefaultRenderGroup(*m_meshNode4);
            addMeshNodeToDefaultRenderGroup(*m_meshNode5);
            m_whiteQuad.GetAppearance().setDrawMode(ramses::EDrawMode_TriangleStrip);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            m_meshNode4->setAppearance(m_whiteQuad.GetAppearance());
            m_meshNode5->setAppearance(m_yellowLine.GetAppearance());
            break;
        case STENCIL_TEST_1:
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Always, 0, 0xff);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Equal, 1, 0xff);
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Equal, 2, 0xff);
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case STENCIL_TEST_2:
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Always, 10, 0x0f);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Replace, ramses::EStencilOperation_Replace, ramses::EStencilOperation_Replace);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Equal, 11, 0x0f); // Stencil is 10, so this should fail
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Decrement, ramses::EStencilOperation_Decrement, ramses::EStencilOperation_Decrement);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Equal, 10, 0x0f); // Stencil is now decremented to 9, so this should also fail
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Keep, ramses::EStencilOperation_Keep, ramses::EStencilOperation_Keep);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case STENCIL_TEST_3:
            // Test decrement wrapping
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_GreaterEqual, 0, 0xff);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_DecrementWrap, ramses::EStencilOperation_DecrementWrap, ramses::EStencilOperation_DecrementWrap);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_Greater, 250, 0xff);
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Decrement, ramses::EStencilOperation_Decrement, ramses::EStencilOperation_Decrement);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc_GreaterEqual, 0, 0xff);
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment, ramses::EStencilOperation_Increment);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case SCISSOR_TEST:
            m_greenTriangle.GetAppearance().setScissorTest(ramses::EScissorTest_Enabled, 90, 50, 50u, 150u);
            m_redTriangle.GetAppearance().setScissorTest(ramses::EScissorTest_Enabled, 90, 50, 50u, 150u);
            m_blueTriangle.GetAppearance().setScissorTest(ramses::EScissorTest_Enabled, 100, 50, 50u, 100u);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case STEREO_RENDERING:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            break;
        case PERSPECTIVE_CAMERA:
        {
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            ramses::PerspectiveCamera* camera = m_scene.createPerspectiveCamera("");
            camera->setFrustum(14.5f, 200.f/200.f, 0.1f, 1500.f);
            camera->setViewport(0, 0, 200, 200);
            setCameraToDefaultRenderPass(camera);
        }
            break;
        case ORTHOGRAPHIC_CAMERA:
        {
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            ramses::OrthographicCamera* camera = m_scene.createOrthographicCamera("");
            camera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
            camera->setViewport(0, 0, 200, 200);

            setCameraToDefaultRenderPass(camera);
        }
            break;
        case THREE_TRIANGLES_WITH_SHARED_COLOR:
        {
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            ramses::DataVector4f* colorData = m_scene.createDataVector4f();
            assert(colorData != NULL);
            colorData->setValue(0.f, 0.f, 0.f, 1.f);
            m_redTriangle.bindColor(*colorData);
            m_greenTriangle.bindColor(*colorData);
            m_blueTriangle.bindColor(*colorData);
            colorData->setValue(1.f, 0.f, 0.f, 1.f);
        }
            break;
        case THREE_TRIANGLES_WITH_UNSHARED_COLOR:
        {
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            ramses::DataVector4f* colorData = m_scene.createDataVector4f();
            assert(colorData != NULL);
            m_redTriangle.bindColor(*colorData);
            m_greenTriangle.bindColor(*colorData);
            m_blueTriangle.bindColor(*colorData);
            colorData->setValue(1.f, 0.f, 0.f, 1.f);
            m_redTriangle.unbindColor();
            m_greenTriangle.unbindColor();
            m_blueTriangle.unbindColor();
            colorData->setValue(0.f, 1.f, 0.f, 1.f);
        }
            break;
        }
    }
}
