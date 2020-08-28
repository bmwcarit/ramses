//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"

#include "ramses-framework-api/DcsmProvider.h"
#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/EDcsmOfferingMode.h"

#include <thread>
#include <iostream>
#include <algorithm>

class DCSMListDemo : ramses::IDcsmProviderEventHandler
{
public:
    DCSMListDemo(int argc, char* argv[])
        : m_framework(argc, argv)
        , m_ramses(*m_framework.createClient("ramses-dcsm-list"))
        , m_dcsm(*m_framework.createDcsmProvider())
        , m_alphaInput()
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "-offer")
            {
                if (argc <= i + 4)
                {
                    std::cerr << "'-offer' argument error: too few arguments, use: -offer sceneId contentId categoryId viewportProviderId" << std::endl;
                    continue;
                }

                int sceneid = atoi(argv[++i]);
                int content = atoi(argv[++i]);
                int category = atoi(argv[++i]);
                int viewportProvider = atoi(argv[++i]);

                if (sceneid > 0)
                    m_sceneToOffer = ramses::sceneId_t(sceneid);
                else
                    std::cerr << "'-offer' argument error: invalid scene, using default " << sceneid << std::endl;

                if (content > 0)
                    m_contentID = ramses::ContentID(content);
                else
                    std::cerr << "'-offer' argument error: invalid content, using default " << m_contentID.getValue() << std::endl;

                if (category > 0)
                    m_categoryID = ramses::Category(category);
                else
                    std::cerr << "'-offer' argument error: invalid category, using default " << m_categoryID.getValue() << std::endl;

                if (viewportProvider > 0)
                    m_viewportProviderID.getReference() = viewportProvider;
                else
                    std::cerr << "'-offer' argument error: invalid viewportProvider, using default " << m_viewportProviderID.getValue() << std::endl;
            }
        }

        m_framework.connect();
    }

    ~DCSMListDemo()
    {
        // shutdown: stop distribution, free resources, unregister
        if (m_scene)
        {
            m_scene->unpublish();
            m_ramses.destroy(*m_scene);
        }

        m_framework.disconnect();
    }

    void createScene()
    {
        // create a scene for distributing content
        m_scene = m_ramses.createScene(m_sceneToOffer, ramses::SceneConfig(), "dcsm list demo scene");

        m_camera = m_scene->createOrthographicCamera("my camera");
        m_camera->setFrustum(0, 735, 0, 50, 0.1f, 100.0f);
        m_camera->setViewport(0u, 0u, 735u, 50u);

        m_cameraViewportSize = m_scene->createDataVector2i();
        m_cameraViewportSize->setValue(735, 50);
        m_camera->bindViewportSize(*m_cameraViewportSize);
        m_scene->createDataProvider(*m_cameraViewportSize, m_viewportProviderID);

        ramses::RenderPass* renderPass = m_scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_Color);
        renderPass->setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        renderPass->setCamera(*m_camera);
        ramses::RenderGroup* renderGroup = m_scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        const float vertexPositionsArray[] = { 0.0f, 0.0f, -1.f, 735.0f, 0.0f, -1.f, 0.0f, 50.0f, -1.f, 735.0f, 50.0f, -1.f };
        const auto vertexPositions = m_scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsArray);

        const float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f };
        const auto textureCoords = m_scene->createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const auto indices = m_scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

        ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-dcsm-list-line.png", *m_scene);
        ramses::TextureSampler* sampler = m_scene->createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-dcsm-list.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-dcsm-list.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

        const ramses::Effect* effectTex = m_scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        m_appearance = m_scene->createAppearance(*effectTex, "triangle appearance");

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene->createGeometryBinding(*effectTex, "triangle geometry");
        geometry->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordsInput;
        effectTex->findAttributeInput("a_position", positionsInput);
        effectTex->findAttributeInput("a_texcoord", texcoordsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texcoordsInput, *textureCoords);

        ramses::UniformInput textureInput;
        effectTex->findUniformInput("textureSampler", textureInput);

        effectTex->findUniformInput("transparency", m_alphaInput);
        m_appearance->setInputTexture(textureInput, *sampler);
        m_appearance->setInputValueFloat(m_alphaInput, 1.0f);
        m_appearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
        m_appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);

        for (int i = 0; i < 10; ++i)
        {
            ramses::MeshNode* meshNode = m_scene->createMeshNode("textured triangle mesh node");
            meshNode->setAppearance(*m_appearance);
            meshNode->setGeometryBinding(*geometry);
            meshNode->translate(0.0f, 50.0f*i, 0.0f);
            renderGroup->addMeshNode(*meshNode, 1);
        }

        m_scene->flush();
        m_scene->publish();
    }

    void runLoop()
    {
        m_sizeReceived = false;
        m_released = false;

        m_dcsm.offerContent(m_contentID, m_categoryID, m_sceneToOffer, ramses::EDcsmOfferingMode::LocalAndRemote);
        while (!m_sizeReceived)
        {
            m_dcsm.dispatchEvents(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds(16u));
        }

        bool currShowing = false;
        ramses::SizeInfo currSize = m_newSize;
        m_camera->setFrustum(0, static_cast<float>(currSize.width), 0, static_cast<float>(currSize.height), 0.1f, 100.0f);
        m_cameraViewportSize->setValue(currSize.width, currSize.height);
        while (!m_released)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16u));
            m_dcsm.dispatchEvents(*this);

            const auto now = uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
            if (currSize != m_newSize)
            {
                const uint32_t listElementHeight = 50;
                const auto sizeStart = !m_sizeAnim.startTime ? now : m_sizeAnim.startTime;
                const auto sizeEnd = !m_sizeAnim.finishTime ? now : m_sizeAnim.finishTime;
                if (sizeStart <= now)
                {
                    const auto currFraction = sizeEnd == sizeStart ? 1.0f : float(now - sizeStart) / (sizeEnd - sizeStart);

                    auto resultSize = m_newSize;
                    if (currFraction >= 0.999f)
                        currSize = m_newSize;
                    else
                    {
                        resultSize.width = static_cast<uint32_t>(currSize.width * (1.f - currFraction) + m_newSize.width * currFraction);
                        resultSize.height = static_cast<uint32_t>(currSize.height * (1.f - currFraction) + m_newSize.height * currFraction);
                    }
                    resultSize.height = (resultSize.height / listElementHeight) * listElementHeight; // cut height to a multiple of listElementHeight
                    m_camera->setFrustum(0, static_cast<float>(resultSize.width), 0, static_cast<float>(resultSize.height), 0.1f, 100.0f);
                    m_cameraViewportSize->setValue(resultSize.width, resultSize.height);

                    m_scene->flush();
                }
            }
            if (currShowing != m_showing)
            {
                const auto showStart = !m_showHideAnim.startTime ? now : m_showHideAnim.startTime;
                const auto showEnd = !m_showHideAnim.finishTime ? now : m_showHideAnim.finishTime;
                if (showStart <= now)
                {
                    const auto currFraction = showEnd == showStart ? 1.0f : float(now - showStart) / (showEnd - showStart);
                    m_appearance->setInputValueFloat(m_alphaInput, m_showing ? std::min(currFraction, 1.f) : 1 - std::min(currFraction, 1.f));
                    if (currFraction >= 0.999f)
                        currShowing = m_showing;

                    m_scene->flush();
                }
            }
        }
    }

    virtual void contentHide(ramses::ContentID /*contentID*/, ramses::AnimationInformation animInfo) override
    {
        m_showing = false;
        m_showHideAnim = animInfo;
    }

    virtual void contentShow(ramses::ContentID /*contentID*/, ramses::AnimationInformation animInfo) override
    {
        m_showing = true;
        m_showHideAnim = animInfo;
    }

    virtual void stopOfferAccepted(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
    {
    }

    virtual void contentSizeChange(ramses::ContentID /*contentID*/, const ramses::CategoryInfoUpdate& categoryInfo, ramses::AnimationInformation animInfo) override
    {
        if (categoryInfo.hasCategorySizeUpdate())
        {
            m_newSize = ramses::SizeInfo{ categoryInfo.getCategorySize().width, categoryInfo.getCategorySize().height };
            m_sizeAnim = animInfo;
            m_sizeReceived = true;
        }
    }

    virtual void contentReadyRequested(ramses::ContentID contentID) override
    {
        m_dcsm.markContentReady(contentID);
    }

    virtual void contentRelease(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
    {
        m_released = true;
        m_showing = false;
    }

private:
    bool m_released = false;
    bool m_showing = false;
    bool m_sizeReceived = false;

    ramses::sceneId_t m_sceneToOffer{123};
    ramses::ContentID m_contentID = ramses::ContentID(101);
    ramses::Category m_categoryID = ramses::Category(1);
    // Viewport size can be linked to data provider on consumer/renderer side.
    // The viewport size represents the content size in screen space
    // and can be used for various layouting effects on consumer/renderer.
    ramses::dataProviderId_t m_viewportProviderID{667};

    ramses::SizeInfo m_newSize{ 0, 0 };
    ramses::AnimationInformation m_sizeAnim = ramses::AnimationInformation{ 0, 0 };
    ramses::AnimationInformation m_showHideAnim = ramses::AnimationInformation{ 0, 0 };

    ramses::RamsesFramework m_framework;
    ramses::RamsesClient& m_ramses;
    ramses::DcsmProvider& m_dcsm;

    ramses::OrthographicCamera* m_camera = nullptr;
    ramses::DataVector2i* m_cameraViewportSize = nullptr;
    ramses::Scene* m_scene = nullptr;
    ramses::Appearance* m_appearance = nullptr;

    ramses::UniformInput m_alphaInput;
};

int main(int argc, char* argv[])
{
    DCSMListDemo demo(argc, argv);
    demo.createScene();
    demo.runLoop();
    return 0;
}
