//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultipleTrianglesScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Camera.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include <cassert>

namespace ramses::internal
{
    MultipleTrianglesScene::MultipleTrianglesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_Effect(createTestEffect(state))
        , m_meshNode1(nullptr)
        , m_meshNode2(nullptr)
        , m_meshNode3(nullptr)
        , m_meshNode4(nullptr)
        , m_meshNode5(nullptr)
        , m_meshNode6(nullptr)
        , m_meshNode7(nullptr)
        , m_whiteTriangle(scene, *m_Effect, TriangleAppearance::EColor::White)
        , m_redTriangle(scene, *m_Effect, TriangleAppearance::EColor::Red)
        , m_greenTriangle(scene, *m_Effect, TriangleAppearance::EColor::Green)
        , m_blueTriangle(scene, *m_Effect, TriangleAppearance::EColor::Blue)
        , m_yellowLine(scene, *m_Effect, Line::EColor_Yellow, ramses::EDrawMode::Lines)
        , m_whiteQuad(scene, *m_Effect, MultiTriangleGeometry::EColor_White)
        , m_triangleFan(scene, *m_Effect, MultiTriangleGeometry::EColor_Red, 1.f, MultiTriangleGeometry::EGeometryType_TriangleFan)
        , m_lineStrip               (scene, *m_Effect, Line::EColor_Red, ramses::EDrawMode::LineStrip)
        , m_linePoints              (scene, *m_Effect, Line::EColor_White, ramses::EDrawMode::Points)
        , m_redTransparentTriangle  (scene, *m_Effect, TriangleAppearance::EColor::Red, 0.6f)
        , m_greenTransparentTriangle(scene, *m_Effect, TriangleAppearance::EColor::Green, 0.6f)
        , m_blueTransparentTriangle (scene, *m_Effect, TriangleAppearance::EColor::Blue, 0.6f)
        , m_colorMaskRedTriangle    (scene, *m_Effect, TriangleAppearance::EColor::White)
        , m_colorMaskGreenTriangle  (scene, *m_Effect, TriangleAppearance::EColor::White)
        , m_colorMaskBlueTriangle   (scene, *m_Effect, TriangleAppearance::EColor::White)
        , m_CCWTriangle             (scene, *m_Effect, TriangleAppearance::EColor::White, 1.f, TriangleGeometry::EVerticesOrder_CCW)
        , m_CWTriangle              (scene, *m_Effect, TriangleAppearance::EColor::White, 1.f, TriangleGeometry::EVerticesOrder_CW)
        , m_CWTriangleCCWIndices    (scene, *m_Effect, TriangleAppearance::EColor::White, 1.f, TriangleGeometry::EVerticesOrder_CW)
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
        m_meshNode6 = m_scene.createMeshNode("trianglefan mesh node");
        m_meshNode7 = m_scene.createMeshNode("line strip mesh node");
        addMeshNodeToDefaultRenderGroup(*m_meshNode1);
        addMeshNodeToDefaultRenderGroup(*m_meshNode2);
        addMeshNodeToDefaultRenderGroup(*m_meshNode3);

        setGeometries(state);
        setTransformations(state);
        setState(state);
    }

    Effect* MultipleTrianglesScene::createTestEffect(uint32_t state)
    {
        if(state == PERSPECTIVE_CAMERA_UBO || state == ORTHOGRAPHIC_CAMERA_UBO || state == CAMERA_TRANSFORMATION_UBO)
            return getTestEffect("ramses-test-client-basic-ubo");
        return getTestEffect("ramses-test-client-basic");
    }

    void MultipleTrianglesScene::setState(uint32_t state)
    {
        ramses::Node* rotate = nullptr;
        ramses::Node* translate = nullptr;

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
            m_redTriangle.GetAppearance().setDepthWrite(ramses::EDepthWrite::Disabled);
            m_greenTriangle.GetAppearance().setDepthWrite(ramses::EDepthWrite::Disabled);
            m_blueTriangle.GetAppearance().setDepthWrite(ramses::EDepthWrite::Disabled);
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 99);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, -1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 1);
            break;
        case ADDITIVE_BLENDING:
            m_redTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::One, ramses::EBlendFactor::One, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_redTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_greenTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::One, ramses::EBlendFactor::One, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_greenTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_blueTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::One, ramses::EBlendFactor::One, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_blueTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_meshNode1->setAppearance(m_redTransparentTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTransparentTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTransparentTriangle.GetAppearance());
            m_meshNode1->setGeometry(m_redTransparentTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_greenTransparentTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_blueTransparentTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case SUBTRACTIVE_BLENDING:
            m_greenTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::One, ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_greenTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::ReverseSubtract, ramses::EBlendOperation::Add);
            m_blueTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::One, ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_blueTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::ReverseSubtract, ramses::EBlendOperation::Add);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTransparentTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTransparentTriangle.GetAppearance());
            m_meshNode1->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_greenTransparentTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_blueTransparentTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case ALPHA_BLENDING:
            m_redTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_redTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_greenTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_greenTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_blueTransparentTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
            m_blueTransparentTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_meshNode1->setAppearance(m_redTransparentTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTransparentTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTransparentTriangle.GetAppearance());
            m_meshNode1->setGeometry(m_redTransparentTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_greenTransparentTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_blueTransparentTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case BLENDING_CONSTANT:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());

            m_whiteTriangle.GetAppearance().setBlendingColor(ramses::vec4f{ .5f, .5f, 0.f, .7f });
            m_whiteTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::ConstColor, ramses::EBlendFactor::ConstAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::Zero);
            m_whiteTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_meshNode2->setAppearance(m_whiteTriangle.GetAppearance());
            m_meshNode2->setGeometry(m_whiteTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            break;
        case BLENDING_DST_COLOR_AND_ALPHA:
            m_meshNode1->setAppearance(m_greenTransparentTriangle.GetAppearance());

            m_whiteTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::DstColor, ramses::EBlendFactor::Zero, ramses::EBlendFactor::One, ramses::EBlendFactor::Zero);
            m_whiteTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_blueTriangle.GetAppearance().setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            m_blueTriangle.GetAppearance().setBlendingFactors(ramses::EBlendFactor::One, ramses::EBlendFactor::Zero, ramses::EBlendFactor::DstAlpha, ramses::EBlendFactor::Zero);
            m_meshNode2->setAppearance(m_whiteTriangle.GetAppearance());
            m_meshNode2->setGeometry(m_whiteTriangle.GetGeometry());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            m_meshNode3->setGeometry(m_blueTriangle.GetGeometry());
            m_meshNode3->translate({0.f, -1.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;

        case COLOR_MASK:
            m_meshNode1->setAppearance(m_colorMaskRedTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_colorMaskGreenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_colorMaskBlueTriangle.GetAppearance());
            m_meshNode1->setGeometry(m_colorMaskRedTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_colorMaskGreenTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_colorMaskBlueTriangle.GetGeometry());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 0);
            break;
        case CAMERA_TRANSFORMATION:
        case CAMERA_TRANSFORMATION_UBO:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());

            rotate = m_scene.createNode();
            translate = m_scene.createNode();

            getDefaultCamera().setParent(*rotate);
            rotate->setParent(*translate);

            translate->setTranslation({-3.f, 3.5f, 0.f});
            rotate->setRotation({-2.0f, -22.0f, -60.0f}, ramses::ERotationType::Euler_XYZ);
            break;
        case FACE_CULLING:
            m_CCWTriangle.GetAppearance().setCullingMode(ramses::ECullMode::BackFacing);
            m_CWTriangle.GetAppearance().setCullingMode(ramses::ECullMode::FrontFacing);
            m_CWTriangleCCWIndices.GetAppearance().setCullingMode(ramses::ECullMode::BackFacing);
            m_meshNode1->setAppearance(m_CCWTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_CWTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_CWTriangleCCWIndices.GetAppearance());
            m_meshNode1->setGeometry(m_CCWTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_CWTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_CWTriangleCCWIndices.GetGeometry());
            break;
        case DEPTH_FUNC:
            m_redTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc::Greater);
            m_greenTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc::Greater);
            m_blueTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc::LessEqual);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 0);
            break;
        case DRAW_MODE:
            m_redTriangle.GetAppearance().setDrawMode(ramses::EDrawMode::Triangles);
            m_yellowLine.GetAppearance().setDrawMode(ramses::EDrawMode::Lines);
            m_greenTriangle.GetAppearance().setDrawMode(ramses::EDrawMode::LineLoop);
            addMeshNodeToDefaultRenderGroup(*m_meshNode4);
            addMeshNodeToDefaultRenderGroup(*m_meshNode5);
            m_whiteQuad.GetAppearance().setDrawMode(ramses::EDrawMode::TriangleStrip);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->removeAppearanceAndGeometry();
            m_meshNode3->setAppearance(m_linePoints.GetAppearance());
            m_meshNode3->setGeometry(m_linePoints.GetGeometry());
            m_meshNode4->setAppearance(m_whiteQuad.GetAppearance());
            m_meshNode5->setAppearance(m_yellowLine.GetAppearance());
            m_meshNode6->setAppearance(m_triangleFan.GetAppearance());
            m_meshNode7->setAppearance(m_lineStrip.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode6);
            addMeshNodeToDefaultRenderGroup(*m_meshNode7);
            break;
        case STENCIL_TEST_1:
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Always, 0, 0xff);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Equal, 1, 0xff);
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Equal, 2, 0xff);
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case STENCIL_TEST_2:
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Always, 10, 0x0f);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Replace, ramses::EStencilOperation::Replace, ramses::EStencilOperation::Replace);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Equal, 11, 0x0f); // Stencil is 10, so this should fail
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Decrement, ramses::EStencilOperation::Decrement, ramses::EStencilOperation::Decrement);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Equal, 10, 0x0f); // Stencil is now decremented to 9, so this should also fail
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Keep, ramses::EStencilOperation::Keep, ramses::EStencilOperation::Keep);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case STENCIL_TEST_3:
            // Test decrement wrapping
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::GreaterEqual, 0, 0xff);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::DecrementWrap, ramses::EStencilOperation::DecrementWrap, ramses::EStencilOperation::DecrementWrap);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Greater, 250, 0xff);
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Decrement, ramses::EStencilOperation::Decrement, ramses::EStencilOperation::Decrement);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::GreaterEqual, 0, 0xff);
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case SCISSOR_TEST:
            m_greenTriangle.GetAppearance().setScissorTest(ramses::EScissorTest::Enabled, 90, 50, 50u, 150u);
            m_redTriangle.GetAppearance().setScissorTest(ramses::EScissorTest::Enabled, 90, 50, 50u, 150u);
            m_blueTriangle.GetAppearance().setScissorTest(ramses::EScissorTest::Enabled, 100, 50, 50u, 100u);
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            addMeshNodeToDefaultRenderGroup(*m_meshNode1, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshNode2, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshNode3, 2);
            break;
        case PERSPECTIVE_CAMERA:
        case PERSPECTIVE_CAMERA_UBO:
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
        case ORTHOGRAPHIC_CAMERA_UBO:
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
            ramses::DataObject* colorData = m_scene.createDataObject(ramses::EDataType::Vector4F);
            assert(colorData != nullptr);
            colorData->setValue(ramses::vec4f{ 0.f, 0.f, 0.f, 1.f });
            m_redTriangle.bindColor(*colorData);
            m_greenTriangle.bindColor(*colorData);
            m_blueTriangle.bindColor(*colorData);
            colorData->setValue(ramses::vec4f{ 1.f, 0.f, 0.f, 1.f });
        }
            break;
        case THREE_TRIANGLES_WITH_UNSHARED_COLOR:
        {
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            ramses::DataObject* colorData = m_scene.createDataObject(ramses::EDataType::Vector4F);
            assert(colorData != nullptr);
            m_redTriangle.bindColor(*colorData);
            m_greenTriangle.bindColor(*colorData);
            m_blueTriangle.bindColor(*colorData);
            colorData->setValue(ramses::vec4f{ 1.f, 0.f, 0.f, 1.f });
            m_redTriangle.unbindColor();
            m_greenTriangle.unbindColor();
            m_blueTriangle.unbindColor();
            colorData->setValue(ramses::vec4f{ 0.f, 1.f, 0.f, 1.f });
        }
            break;
        case EULER_ROTATION_CONVENTIONS:
            m_meshNode1->setAppearance(m_redTriangle.GetAppearance());
            m_meshNode2->setAppearance(m_greenTriangle.GetAppearance());
            m_meshNode3->setAppearance(m_blueTriangle.GetAppearance());
            m_meshNode4->setAppearance(m_whiteTriangle.GetAppearance());
            m_meshNode5->setAppearance(m_redTransparentTriangle.GetAppearance());
            m_meshNode6->setAppearance(m_greenTransparentTriangle.GetAppearance());

            addMeshNodeToDefaultRenderGroup(*m_meshNode4);
            addMeshNodeToDefaultRenderGroup(*m_meshNode5);
            addMeshNodeToDefaultRenderGroup(*m_meshNode6);
            break;
        default:
            break;
        }

        if (state == ORTHOGRAPHIC_CAMERA_UBO || state == PERSPECTIVE_CAMERA_UBO || state == CAMERA_TRANSFORMATION_UBO)
        {
            const auto uniform = m_redTriangle.GetAppearance().getEffect().findUniformInput("generalUbo.variant");
            assert(uniform);
            m_redTriangle.GetAppearance().setInputValue(*uniform, 1);
            m_greenTriangle.GetAppearance().setInputValue(*uniform, 2);
            m_blueTriangle.GetAppearance().setInputValue(*uniform, 3);
        }
    }

    void MultipleTrianglesScene::setGeometries(uint32_t state)
    {
        if (state == EULER_ROTATION_CONVENTIONS)
        {
            //use same geometry for all meshes
            m_meshNode1->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode4->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode5->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode6->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode7->setGeometry(m_redTriangle.GetGeometry());
        }
        else
        {
            m_meshNode1->setGeometry(m_redTriangle.GetGeometry());
            m_meshNode2->setGeometry(m_greenTriangle.GetGeometry());
            m_meshNode3->setGeometry(m_blueTriangle.GetGeometry());
            m_meshNode4->setGeometry(m_whiteQuad.GetGeometry());
            m_meshNode5->setGeometry(m_yellowLine.GetGeometry());
            m_meshNode6->setGeometry(m_triangleFan.GetGeometry());
            m_meshNode7->setGeometry(m_lineStrip.GetGeometry());
        }
    }

    void MultipleTrianglesScene::setTransformations(uint32_t state)
    {
        ramses::Node* transNode1 = m_scene.createNode();
        ramses::Node* transNode2 = m_scene.createNode();
        ramses::Node* transNode3 = m_scene.createNode();
        ramses::Node* transNode4 = m_scene.createNode();
        ramses::Node* transNode5 = m_scene.createNode();
        ramses::Node* transNode6 = m_scene.createNode();

        if (state == EULER_ROTATION_CONVENTIONS)
        {
            transNode1->setTranslation({-1.f, -1.f, -15.f});
            transNode2->setTranslation({-1.f, 0.f, -15.f});
            transNode3->setTranslation({-1.f, 1.f, -15.f});
            transNode4->setTranslation({1.f, -1.f, -15.f});
            transNode5->setTranslation({1.f, 0.f, -15.f});
            transNode6->setTranslation({1.f, 1.f, -15.f});

            transNode1->setRotation({0.f, 0.f, 45.f}, ramses::ERotationType::Euler_ZYX);
            transNode2->setRotation({0.f, 60.f, 45.f}, ramses::ERotationType::Euler_ZYX);
            transNode3->setRotation({60.f, 60.f, 45.f}, ramses::ERotationType::Euler_ZYX);
            transNode4->setRotation({45.f, 60.f, 45.f}, ramses::ERotationType::Euler_ZYZ);

            ramses::Node* transNode5Child = m_scene.createNode();
            transNode5->addChild(*transNode5Child);
            transNode5->setRotation({60.f, 0.f, 0.f}, ramses::ERotationType::Euler_XYZ);
            transNode5Child->setRotation({0.f, 60.f, 45.f}, ramses::ERotationType::Euler_XZY);

            ramses::Node* transNode6Child = m_scene.createNode();
            transNode6->addChild(*transNode6Child);
            transNode6->setRotation({-80.f, -60.f, -45.f}, ramses::ERotationType::Euler_XYZ);
            transNode6Child->setRotation({80.f, 60.f, 45.f}, ramses::ERotationType::Euler_ZYX);

            m_meshNode1->setParent(*transNode1);
            m_meshNode2->setParent(*transNode2);
            m_meshNode3->setParent(*transNode3);
            m_meshNode4->setParent(*transNode4);
            m_meshNode5->setParent(*transNode5Child);
            m_meshNode6->setParent(*transNode6Child);
        }
        else
        {
            transNode1->setTranslation({0.f, -0.2f, -12.f});
            transNode2->setTranslation({-0.2f, 0.f, -11.f});
            transNode3->setTranslation({0.2f, 0.2f, -10.f});
            transNode4->setTranslation({-0.2f, -0.2f, -9.f});
            transNode5->setTranslation({-0.3f, -0.2f, -8.f});
            transNode6->setTranslation({2.0f, -0.6f, -12.f});

            ramses::Node* transNode7 = m_scene.createNode();
            transNode7->setTranslation({1.0f, 0.6f, -12.f});

            m_meshNode1->setParent(*transNode1);
            m_meshNode2->setParent(*transNode2);
            m_meshNode3->setParent(*transNode3);
            m_meshNode4->setParent(*transNode4);
            m_meshNode5->setParent(*transNode5);
            m_meshNode6->setParent(*transNode6);
            m_meshNode7->setParent(*transNode7);
        }
    }
}
