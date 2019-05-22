//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/FileLoadingScene.h"
#include "ramses-utils.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/UniformInput.h"
#include "RamsesObjectTypeUtils.h"
#include "TextureUtils.h"
#include "Math3d/Vector3.h"

namespace ramses_internal
{
    FileLoadingScene::FileLoadingScene(ramses::RamsesClient& clientForLoading, UInt32 state, ramses::sceneId_t sceneId, const Vector3& cameraPosition, const String& folder, const ramses::RamsesFrameworkConfig& config)
        : m_createdScene(NULL)
    {
        switch (state)
        {
        case CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT:
        {
            ramses::RamsesFramework separateFramework(config);
            ramses::RamsesClient separateClient("ramses-test-client-fileLoadingScene-createFiles", separateFramework);
            createFiles(separateClient, sceneId, cameraPosition, folder);
            loadFromFiles(clientForLoading, folder);
            break;
        }
        case CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT:
        {
            createFiles(clientForLoading, sceneId, cameraPosition, folder);
            loadFromFiles(clientForLoading, folder);
            break;
        }
        default:
            assert(false && "Invalid state for FileLoadingScene");
            break;
        }
    }

    void FileLoadingScene::createFiles(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const Vector3& cameraPosition, const String& folder, const ramses::SceneConfig& sceneConfig)
    {
        ramses::Scene* scene = client.createScene(sceneId, sceneConfig);

        ramses::Node* cameraTranslation = scene->createNode("cameraPosition");
        cameraTranslation->setTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ramses::Camera* camera = scene->createRemoteCamera("my camera");
        camera->setParent(*cameraTranslation);
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup("render group");
        renderPass->addRenderGroup(*renderGroup);

        ramses::ResourceFileDescriptionSet resourceFileInformation;
        ramses::ResourceFileDescription textureAssets((folder + String("/texture.ramres")).c_str());
        ramses::ResourceFileDescription triangleAssets((folder + String("/triangle.ramres")).c_str());

        float vertexPositionsArray[] = { -0.5f, -0.5f, -1.f, 0.5f, -0.5f, -1.f, -0.5f, 0.5f, -1.f, 0.5f, 0.5f, -1.f };
        const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(4, vertexPositionsArray);
        textureAssets.add(vertexPositions);

        float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f };
        const ramses::Vector2fArray* textureCoords = client.createConstVector2fArray(4, textureCoordsArray);
        textureAssets.add(textureCoords);

        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = client.createConstUInt16Array(6, indicesArray);
        ramses::Texture2D* texture = ramses::TextureUtils::CreateTextureResourceFromPng("res/ramses-test-client-file-loading-texture.png", client);
        assert(texture != NULL);

        textureAssets.add(indices);
        textureAssets.add(texture);

        ramses::TextureSampler* sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest,
            ramses::ETextureSamplingMethod_Nearest,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-file-loading-texturing.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-file-loading-texturing.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

        const ramses::Effect* effectTex = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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
        renderGroup->addMeshNode(*meshNode);

        scaleNode->addChild(*meshNode);

        initializeAnimationContent(client, *scene, triangleAssets, *renderGroup);

        resourceFileInformation.add(textureAssets);
        resourceFileInformation.add(triangleAssets);
        client.saveSceneToFile(*scene, (folder + String("/tempfile.ramses")).c_str(), resourceFileInformation, false);

        client.destroy(*scene);

        for (uint32_t i = 0; i < resourceFileInformation.getNumberOfDescriptions(); ++i)
        {
            const ramses::ResourceFileDescription& resDesc = resourceFileInformation.getDescription(i);

            for (uint32_t j = 0; j < resDesc.getNumberOfResources(); ++j)
            {
                const ramses::Resource& resource = resDesc.getResource(j);
                client.destroy(resource);
            }
        }
    }

    void FileLoadingScene::initializeAnimationContent(ramses::RamsesClient& ramses, ramses::Scene& scene, ramses::ResourceFileDescription& resources, ramses::RenderGroup& renderGroup)
    {
        // prepare triangle geometry: vertex position array and index array
        float vertexPositionsData[] = { -0.3f, 0.f, -0.3f, 0.3f, 0.f, -0.3f, 0.f, 0.3f, -0.3f };
        const ramses::Vector3fArray* vertexPositions = ramses.createConstVector3fArray(3, vertexPositionsData);
        resources.add(vertexPositions);
        uint16_t indexData[] = { 0, 1, 2 };
        const ramses::UInt16Array* indices = ramses.createConstUInt16Array(3, indexData);
        resources.add(indices);

        // create an appearance for red triangle
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-file-loading-basic.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-file-loading-red.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

        const ramses::Effect* effect = ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader anim");
        resources.add(effect);
        ramses::Appearance* appearance = scene.createAppearance(*effect, "triangle appearance anim");

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = scene.createGeometryBinding(*effect, "triangle geometry");
        geometry->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        effect->findAttributeInput("a_position", positionsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);

        // create a mesh nodes to define the triangles with chosen appearance
        ramses::MeshNode* meshNode1 = scene.createMeshNode("red triangle mesh node1");
        meshNode1->setAppearance(*appearance);
        meshNode1->setGeometryBinding(*geometry);
        ramses::MeshNode* meshNode2 = scene.createMeshNode("red triangle mesh node2");
        meshNode2->setAppearance(*appearance);
        meshNode2->setGeometryBinding(*geometry);
        ramses::MeshNode* meshNode3 = scene.createMeshNode("red triangle mesh node3");
        meshNode3->setAppearance(*appearance);
        meshNode3->setGeometryBinding(*geometry);

        // mesh needs to be added to a render pass with camera in order to be rendered
        renderGroup.addMeshNode(*meshNode1);
        renderGroup.addMeshNode(*meshNode2);
        renderGroup.addMeshNode(*meshNode3);

        // create a translation node for each mesh node
        ramses::Node* transNode1 = scene.createNode();
        ramses::Node* transNode2 = scene.createNode();
        ramses::Node* transNode3 = scene.createNode();
        meshNode1->setParent(*transNode1);
        meshNode2->setParent(*transNode2);
        meshNode3->setParent(*transNode3);

        // create animation system
        ramses::AnimationSystem* animationSystem = scene.createAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");

        // create splines with animation keys
        ramses::SplineLinearFloat* spline1 = animationSystem->createSplineLinearFloat("spline1");
        spline1->setKey(0u, 0.f);
        spline1->setKey(5000u, -1.f);
        spline1->setKey(10000u, 0.f);
        ramses::SplineLinearFloat* spline2 = animationSystem->createSplineLinearFloat("spline2");
        spline2->setKey(0u, 0.f);
        spline2->setKey(5000u, 1.f);
        spline2->setKey(10000u, 0.f);

        // create animated property for each translation node with single component animation
        ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*transNode1, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
        ramses::AnimatedProperty* animProperty2 = animationSystem->createAnimatedProperty(*transNode2, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
        ramses::AnimatedProperty* animProperty3 = animationSystem->createAnimatedProperty(*transNode3, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_Y);

        // create three animations
        ramses::Animation* animation1 = animationSystem->createAnimation(*animProperty1, *spline1, "animation1");
        ramses::Animation* animation2 = animationSystem->createAnimation(*animProperty2, *spline2, "animation2");
        ramses::Animation* animation3 = animationSystem->createAnimation(*animProperty3, *spline1, "animation3"); // we can reuse spline1 for animating Y component of the third translation node

        // create animation sequence
        ramses::AnimationSequence* animSequence = animationSystem->createAnimationSequence();

        // add animations to a sequence
        animSequence->addAnimation(*animation1);
        animSequence->addAnimation(*animation2);
        animSequence->addAnimation(*animation3);

        // set animation properties (optional)
        animSequence->setAnimationLooping(*animation1);
        animSequence->setAnimationLooping(*animation2);
        animSequence->setAnimationLooping(*animation3);

        // set playbackSpeed
        animSequence->setPlaybackSpeed(5.f);

        // start animation sequence
        animSequence->startAt(0u);

        animationSystem->setTime(20800u);
    }

    void FileLoadingScene::loadFromFiles(ramses::RamsesClient& ramses, const String& folder)
    {
        ramses::ResourceFileDescriptionSet resourceFileInformation;
        resourceFileInformation.add(ramses::ResourceFileDescription((folder + String("/texture.ramres")).c_str()));
        resourceFileInformation.add(ramses::ResourceFileDescription((folder + String("/triangle.ramres")).c_str()));

        ramses::Scene* loadedScene = ramses.loadSceneFromFile((folder + String("/tempfile.ramses")).c_str(), resourceFileInformation);

        // make changes to loaded scene
        ramses::Node& loadedScaleNode = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Node>(*loadedScene->findObjectByName("scale node"));
        loadedScaleNode.setScaling(2, 2, 2);
        loadedScene->flush();

        ramses::AnimationSystem& loadedAnimSystem = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::AnimationSystem>(*loadedScene->findObjectByName("animation system"));
        const ramses::globalTimeStamp_t currTimeState = loadedAnimSystem.getTime();
        loadedAnimSystem.setTime(currTimeState + 3333u);

        loadedScene->flush();
        m_createdScene = loadedScene;
    }

    ramses::Scene* FileLoadingScene::getCreatedScene()
    {
        return m_createdScene;
    }
}
