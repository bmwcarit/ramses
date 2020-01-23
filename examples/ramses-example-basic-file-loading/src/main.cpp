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

int main(int argc, char* argv[])
{
    // create a scene and write it to a file
    {
        ramses::RamsesFramework framework(argc, argv);
        ramses::RamsesClient& ramses(*framework.createClient("ramses-example-file-loading"));
        ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(23u), ramses::SceneConfig(), "basic scene loading from file");
        // every scene needs a render pass with camera
        ramses::Camera* camera = scene->createRemoteCamera("my camera");
        camera->setTranslation(0.0f, 0.0f, 5.0f);
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);

        ramses::ResourceFileDescriptionSet resourceFileInformation;
        ramses::ResourceFileDescription textureAssets( "res/texture.ramres");
        ramses::ResourceFileDescription triangleAssets( "res/triangle.ramres" );

        float vertexPositionsArray[] = { -0.5f, -0.5f, -1.f, 0.5f, -0.5f, -1.f, -0.5f, 0.5f, -1.f, 0.5f, 0.5f, -1.f };
        const ramses::Vector3fArray* vertexPositions = ramses.createConstVector3fArray(4, vertexPositionsArray);
        textureAssets.add( vertexPositions );

        float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f };
        const ramses::Vector2fArray* textureCoords = ramses.createConstVector2fArray(4, textureCoordsArray);
        textureAssets.add( textureCoords );

        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = ramses.createConstUInt16Array(6, indicesArray);
        ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-basic-file-loading-texture.png", ramses);

        textureAssets.add( indices );
        textureAssets.add( texture );

        ramses::TextureSampler* sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-example-basic-file-loading-texturing.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-file-loading-texturing.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

        const ramses::Effect* effectTex = ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        textureAssets.add(effectTex);

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

        resourceFileInformation.add( textureAssets );
        resourceFileInformation.add( triangleAssets );
        ramses.saveSceneToFile(*scene, "tempfile.ramses", resourceFileInformation, false);

        ramses.destroy(*scene);
        ramses.destroy(*vertexPositions);
        ramses.destroy(*textureCoords);
        ramses.destroy(*indices);
    }

    // load the saved file
    {
        ramses::RamsesFramework framework(argc, argv);
        ramses::RamsesClient& ramses(*framework.createClient("ramses-example-file-loading"));

        /// [Basic File Loading Example]
        // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
        //                 This should not be the case for real applications.
        ramses::ResourceFileDescriptionSet resourceFileInformation;
        resourceFileInformation.add(ramses::ResourceFileDescription("res/texture.ramres"));
        resourceFileInformation.add(ramses::ResourceFileDescription("res/triangle.ramres"));

        ramses::Scene* loadedScene = ramses.loadSceneFromFile("tempfile.ramses", resourceFileInformation);

        // make changes to loaded scene
        ramses::RamsesObject* loadedObject = loadedScene->findObjectByName("scale node");
        ramses::Node* loadedScaleNode = ramses::RamsesUtils::TryConvert<ramses::Node>(*loadedObject);
        /// [Basic File Loading Example]

        framework.connect();

        loadedScene->publish();

        loadedScaleNode->setScaling(2, 2, 2);

        loadedScene->flush();
        std::this_thread::sleep_for(std::chrono::seconds(30));

        loadedScene->unpublish();
        ramses.destroy(*loadedScene);
        framework.disconnect();
    }
    return 0;
}
