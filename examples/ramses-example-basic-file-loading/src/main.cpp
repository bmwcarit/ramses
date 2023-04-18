//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"
#include <thread>

/**
 * @example ramses-example-basic-file-loading/src/main.cpp
 * @brief Basic File Loading Example
 */

int main()
{
    // create a scene and write it to a file
    {
        ramses::RamsesFramework framework;
        ramses::RamsesClient& ramses(*framework.createClient("ramses-example-file-loading"));
        ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "basic scene loading from file");
        // every scene needs a render pass with camera
        auto* camera = scene->createPerspectiveCamera("my camera");
        camera->setViewport(0, 0, 1280u, 480u);
        camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
        camera->setTranslation({0.0f, 0.0f, 5.0f});
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_None);
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
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-example-basic-file-loading-texturing.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-file-loading-texturing.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* effectTex = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

        ramses::Appearance* appearance = scene->createAppearance(*effectTex, "triangle appearance");
        ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effectTex, "triangle geometry");

        geometry->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordsInput;
        effectTex->findAttributeInput("a_position", positionsInput);
        effectTex->findAttributeInput("a_texcoord", texcoordsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texcoordsInput, *textureCoords);

        ramses::UniformInput textureInput;
        effectTex->findUniformInput("textureSampler", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::Node* scaleNode = scene->createNode("scale node");

        ramses::MeshNode* meshNode = scene->createMeshNode("textured triangle mesh node");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);
        // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
        renderGroup->addMeshNode(*meshNode);

        scaleNode->addChild(*meshNode);

        scene->saveToFile("tempfile.ramses", false);

        scene->destroy(*vertexPositions);
        scene->destroy(*textureCoords);
        scene->destroy(*indices);
        ramses.destroy(*scene);
    }

    // load the saved file
    {
        ramses::RamsesFramework framework;
        ramses::RamsesClient& ramses(*framework.createClient("ramses-example-file-loading"));

        /// [Basic File Loading Example]
        // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
        //                 This should not be the case for real applications.
        ramses::Scene* loadedScene = ramses.loadSceneFromFile("tempfile.ramses");

        // make changes to loaded scene
        ramses::RamsesObject* loadedObject = loadedScene->findObjectByName("scale node");
        ramses::Node* loadedScaleNode = ramses::RamsesUtils::TryConvert<ramses::Node>(*loadedObject);
        /// [Basic File Loading Example]

        framework.connect();

        loadedScene->publish();

        loadedScaleNode->setScaling({2, 2, 2});

        loadedScene->flush();
        std::this_thread::sleep_for(std::chrono::seconds(30));

        loadedScene->unpublish();
        ramses.destroy(*loadedScene);
        framework.disconnect();
    }
    return 0;
}
