//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"
#include <thread>

/**
 * @example ramses-example-basic-file-loading/src/main.cpp
 * @brief Basic File Loading Example
 */

int main()
{
    bool status = false;
    // create a scene and write it to a file
    {
        ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework(config);
        ramses::RamsesClient& ramses(*framework.createClient("ramses-example-file-loading"));
        const ramses::SceneConfig sceneConfig(ramses::sceneId_t{123});
        ramses::Scene* scene = ramses.createScene(sceneConfig, "basic scene loading from file");
        // every scene needs a render pass with camera
        auto* camera = scene->createPerspectiveCamera("my camera");
        camera->setViewport(0, 0, 1280u, 480u);
        camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
        camera->setTranslation({0.0f, 0.0f, 5.0f});
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlag::None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{ ramses::vec3f{-0.5f, -0.5f, -1.f}, ramses::vec3f{0.5f, -0.5f, -1.f}, ramses::vec3f{-0.5f, 0.5f, -1.f}, ramses::vec3f{0.5f, 0.5f, -1.f} };
        ramses::ArrayResource* vertexPositions = scene->createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
        ramses::ArrayResource* textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

        const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
        ramses::ArrayResource* indices = scene->createArrayResource(6u, indicesArray.data());

        ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-basic-file-loading-texture.png", *scene);
        ramses::TextureSampler* sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear,
            ramses::ETextureSamplingMethod::Linear,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-example-basic-file-loading-texturing.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-file-loading-texturing.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* effectTex = scene->createEffect(effectDesc, "glsl shader");

        ramses::Appearance* appearance = scene->createAppearance(*effectTex, "triangle appearance");
        ramses::Geometry* geometry = scene->createGeometry(*effectTex, "triangle geometry");

        geometry->setIndices(*indices);
        std::optional<ramses::AttributeInput> positionsInput = effectTex->findAttributeInput("a_position");
        std::optional<ramses::AttributeInput> texcoordsInput = effectTex->findAttributeInput("a_texcoord");
        std::optional<ramses::UniformInput>   textureInput   = effectTex->findUniformInput("textureSampler");
        assert(positionsInput.has_value() && texcoordsInput.has_value() && textureInput.has_value());
        geometry->setInputBuffer(*positionsInput, *vertexPositions);
        geometry->setInputBuffer(*texcoordsInput, *textureCoords);
        appearance->setInputTexture(*textureInput, *sampler);

        ramses::Node* scaleNode = scene->createNode("scale node");

        ramses::MeshNode* meshNode = scene->createMeshNode("textured triangle mesh node");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);
        // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
        renderGroup->addMeshNode(*meshNode);

        scaleNode->addChild(*meshNode);

        status = scene->saveToFile("tempfile.ramses", {});

        scene->destroy(*vertexPositions);
        scene->destroy(*textureCoords);
        scene->destroy(*indices);
        ramses.destroy(*scene);
    }

    if (!status)
    {
        return EXIT_FAILURE;
    }

    // load the saved file
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework       framework(config);
    ramses::RamsesClient&         ramses(*framework.createClient("ramses-example-file-loading"));

    /// [Basic File Loading Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.
    // Load scene with remote publication enabled.
    ramses::Scene* loadedScene = ramses.loadSceneFromFile("tempfile.ramses", ramses::SceneConfig({}, ramses::EScenePublicationMode::LocalAndRemote));

    // make changes to loaded scene
    auto* loadedScaleNode = loadedScene->findObject<ramses::Node>("scale node");
    /// [Basic File Loading Example]

    framework.connect();

    loadedScene->publish(ramses::EScenePublicationMode::LocalAndRemote);

    loadedScaleNode->setScaling({2, 2, 2});

    loadedScene->flush();
    std::this_thread::sleep_for(std::chrono::seconds(30));

    loadedScene->unpublish();
    ramses.destroy(*loadedScene);
    framework.disconnect();

    return 0;
}
