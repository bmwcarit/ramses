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
#include <fstream>

/**
 * @example ramses-example-dcsm-provider/src/main.cpp
 * @brief DCSM Provider Example
 */

/**
 * DCSM Provider class
 */
class DCSMProviderExample : ramses::IDcsmProviderEventHandler
{
public:
    /**
     * @brief Example constructor
     * @param[in] argc executable argument count
     * @param[in] argv executable argument values
     */
    DCSMProviderExample(int argc, char* argv[])
        : m_framework(argc, argv)
        , m_ramses(*m_framework.createClient("ramses-example-dcsm-provider"))
        , m_dcsm(*m_framework.createDcsmProvider())
        , m_alphaInput()
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "-offer")
            {
                if (argc <= i + 3)
                {
                    std::cerr << "'-offer' argument error: too few arguments" << std::endl;
                    continue;
                }

                int sceneid = atoi(argv[++i]);
                int content = atoi(argv[++i]);
                int category = atoi(argv[++i]);

                if (sceneid > 0)
                    m_sceneToOffer = ramses::sceneId_t(sceneid);
                else
                    std::cerr << "'-offer' argument error: invalid scene, using default" << std::endl;

                if (content > 0)
                    m_contentID = ramses::ContentID(content);
                else
                    std::cerr << "'-offer' argument error: invalid content, using default" << std::endl;

                if (category > 0)
                    m_categoryID = ramses::Category(category);
                else
                    std::cerr << "'-offer' argument error: invalid category, using default" << std::endl;
            }
        }

        m_framework.connect();
    }

    ~DCSMProviderExample()
    {
        // shutdown: stop distribution, free resources, unregister
        if (m_scene)
        {
            m_scene->unpublish();
            m_ramses.destroy(*m_scene);
        }

        m_framework.disconnect();
    }

    /**
     * @brief Create the scene that the example provides via DCSM
     */
    void createScene()
    {
        // create a scene for distributing content
        m_scene = m_ramses.createScene(m_sceneToOffer, ramses::SceneConfig(), "dcsm example scene");

        m_camera = m_scene->createOrthographicCamera("my camera");
        m_camera->setFrustum(-1, 1, -1, 1, 0.1f, 100.0f);
        m_camera->setViewport(0u, 0u, 100u, 100u);

        ramses::RenderPass* renderPass = m_scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_Color);
        renderPass->setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        renderPass->setCamera(*m_camera);
        ramses::RenderGroup* renderGroup = m_scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        const float vertexPositionsArray[] = { -1.0f, -1.0f, -1.f, 1.0f, -1.0f, -1.f, -1.0f, 1.0f, -1.f, 1.0f, 1.0f, -1.f };
        const ramses::Vector3fArray* vertexPositions = m_ramses.createConstVector3fArray(4, vertexPositionsArray);

        const float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f };
        const ramses::Vector2fArray* textureCoords = m_ramses.createConstVector2fArray(4, textureCoordsArray);

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = m_ramses.createConstUInt16Array(6, indicesArray);

        const ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-dcsm-provider-texture.png", m_ramses);
        const ramses::TextureSampler* sampler = m_scene->createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-example-dcsm-provider.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-example-dcsm-provider.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
        ramses::EffectDescription effectDescOutline;
        effectDescOutline.setVertexShaderFromFile("res/ramses-example-dcsm-provider-outline.vert");
        effectDescOutline.setFragmentShaderFromFile("res/ramses-example-dcsm-provider-outline.frag");
        effectDescOutline.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

        const ramses::Effect* effectTex = m_ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        m_appearance = m_scene->createAppearance(*effectTex, "triangle appearance");
        const ramses::Effect* effectOutline = m_ramses.createEffect(effectDescOutline, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        ramses::Appearance* appearanceOutline = m_scene->createAppearance(*effectOutline, "outline appearance");

        ramses::GeometryBinding* geometry = m_scene->createGeometryBinding(*effectTex, "triangle geometry");
        ramses::GeometryBinding* geometryOutline = m_scene->createGeometryBinding(*effectOutline, "triangle geometry");
        geometry->setIndices(*indices);
        geometryOutline->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordsInput;
        effectTex->findAttributeInput("a_position", positionsInput);
        effectTex->findAttributeInput("a_texcoord", texcoordsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texcoordsInput, *textureCoords);

        effectOutline->findAttributeInput("a_position", positionsInput);
        effectOutline->findAttributeInput("a_texcoord", texcoordsInput);
        geometryOutline->setInputBuffer(positionsInput, *vertexPositions);
        geometryOutline->setInputBuffer(texcoordsInput, *textureCoords);

        ramses::UniformInput textureInput;
        effectTex->findUniformInput("textureSampler", textureInput);

        ramses::UniformInput alphaInput;
        effectTex->findUniformInput("transparency", m_alphaInput);
        m_appearance->setInputTexture(textureInput, *sampler);
        m_appearance->setInputValueFloat(m_alphaInput, 1.0f);
        m_appearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
        m_appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);

        effectOutline->findUniformInput("textureSampler", textureInput);
        appearanceOutline->setInputTexture(textureInput, *sampler);

        ramses::MeshNode* meshNode = m_scene->createMeshNode("textured triangle mesh node");
        ramses::MeshNode* meshSizeOutline = m_scene->createMeshNode("size outline");

        meshSizeOutline->setAppearance(*appearanceOutline);
        meshSizeOutline->setGeometryBinding(*geometryOutline);
        meshNode->setAppearance(*m_appearance);
        meshNode->setGeometryBinding(*geometry);

        renderGroup->addMeshNode(*meshNode, 1);
        renderGroup->addMeshNode(*meshSizeOutline, 2);

        m_scene->flush();
        m_scene->publish();
    }

    /**
     * @brief Load image data from file to use as preview image
     * @return The data from the whole file
     */
    std::vector<uint8_t> loadPreviewImageFile()
    {
        // load preview image
        std::ifstream file;
        file.open("res/ramses-example-dcsm-provider-texture.png", std::fstream::binary | std::fstream::in);
        if (!file.good())
        {
            printf("Could not open preview image file!\n");
            exit(1);
        }
        file.seekg(0, std::ios::end);
        const auto numberOfBytes = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> fileContents(static_cast<unsigned int>(numberOfBytes));
        file.read(reinterpret_cast<char*>(fileContents.data()), static_cast<std::streamsize>(numberOfBytes));
        file.close();
        return fileContents;
    }

    /**
     * @brief The example main loop
     */
    void runLoopForOneLifecycle()
    {
        m_sizeReceived = false;
        m_released = false;

        // first is to offer the content to a consumer listening
        ramses::DcsmMetadataCreator metadata;
        metadata.setPreviewDescription(std::u32string(U"example/пример/例"));
        const auto& fileContents = loadPreviewImageFile();
        metadata.setPreviewImagePng(fileContents.data(), fileContents.size());
        m_dcsm.offerContentWithMetadata(m_contentID, m_categoryID, m_sceneToOffer, ramses::EDcsmOfferingMode::LocalAndRemote, metadata);

        // wait for the consumer accepting the offer and sending the initial size
        while (!m_sizeReceived)
        {
            m_dcsm.dispatchEvents(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds(16u));
        }

        bool currShowing = false;
        ramses::SizeInfo currSize = m_newSize;
        m_camera->setViewport(0u, 0u, currSize.width, currSize.height);
        while (!m_released)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16u));

            // dispatch the events coming in via DCSM from the consumer
            m_dcsm.dispatchEvents(*this);

            const auto now = uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
            if (currSize != m_newSize)
            {
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
                    m_camera->setViewport(0u, 0u, resultSize.width, resultSize.height);

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

    // DCSM Provider event handler interface implementation
    virtual void contentHide(ramses::ContentID /*contentID*/, ramses::AnimationInformation animInfo) override
    {
        // save the hide parameters, which are then used within our logic loop
        m_showing = false;
        m_showHideAnim = animInfo;
    }

    // DCSM Provider event handler interface implementation
    virtual void contentShow(ramses::ContentID /*contentID*/, ramses::AnimationInformation animInfo) override
    {
        // save the show parameters, which are then used within our logic loop
        m_showing = true;
        m_showHideAnim = animInfo;
    }

    // DCSM Provider event handler interface implementation
    virtual void stopOfferAccepted(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
    {
        // we will never request the content to be not offered anymore in this example, so this is not needed
    }

    // DCSM Provider event handler interface implementation
    virtual void contentSizeChange(ramses::ContentID /*contentID*/, const ramses::CategoryInfoUpdate& categoryInfo, ramses::AnimationInformation animInfo) override
    {
        // save the resize parameters, which are then used within our logic loop
        m_newSize = ramses::SizeInfo{ categoryInfo.getCategorySize().width, categoryInfo.getCategorySize().height };
        m_sizeAnim = animInfo;
        m_sizeReceived = true;
    }

    // DCSM Provider event handler interface implementation
    virtual void contentReadyRequested(ramses::ContentID contentID) override
    {
        // we have our content ready before we offer it, so we automatically reply with ready here
        m_dcsm.markContentReady(contentID);
    }

    // DCSM Provider event handler interface implementation
    virtual void contentRelease(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
    {
        // if the consumer requests the content to be gone, we will leave our logic loop
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

    ramses::SizeInfo m_newSize = ramses::SizeInfo{ 0, 0 };
    ramses::AnimationInformation m_sizeAnim = ramses::AnimationInformation{ 0, 0 };
    ramses::AnimationInformation m_showHideAnim = ramses::AnimationInformation{ 0, 0 };

    ramses::RamsesFramework m_framework;
    ramses::RamsesClient& m_ramses;
    ramses::DcsmProvider& m_dcsm;

    ramses::Scene* m_scene = nullptr;
    ramses::OrthographicCamera* m_camera = nullptr;
    ramses::Appearance* m_appearance = nullptr;
    ramses::UniformInput m_alphaInput;
};

int main(int argc, char* argv[])
{
    DCSMProviderExample example(argc, argv);
    example.createScene();
    example.runLoopForOneLifecycle();
    return 0;
}
