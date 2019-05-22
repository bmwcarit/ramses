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

void initializeAnimationContent(ramses::RamsesClient& ramses, ramses::Scene& scene, ramses::ResourceFileDescription& resources, ramses::RenderGroup& renderGroup)
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
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-file-loading-basic.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-file-loading-red.frag");
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

    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup.addMeshNode(*meshNode1);
    renderGroup.addMeshNode(*meshNode2);
    renderGroup.addMeshNode(*meshNode3);

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
    ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*meshNode1, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
    ramses::AnimatedProperty* animProperty2 = animationSystem->createAnimatedProperty(*meshNode2, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
    ramses::AnimatedProperty* animProperty3 = animationSystem->createAnimatedProperty(*meshNode3, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_Y);

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

int main(int argc, char* argv[])
{
    // create a scene and write it to a file
    {
        ramses::RamsesFramework framework(argc, argv);
        ramses::RamsesClient ramses("ramses-example-file-loading", framework);
        ramses::Scene* scene = ramses.createScene(23u, ramses::SceneConfig(), "basic scene loading from file");
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

        initializeAnimationContent(ramses, *scene, triangleAssets, *renderGroup);

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
        ramses::RamsesClient ramses("ramses-example-file-loading", framework);

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

        ramses::AnimationSystem* loadedAnimSystem = ramses::RamsesUtils::TryConvert<ramses::AnimationSystem>(*loadedScene->findObjectByName("animation system"));
        const ramses::globalTimeStamp_t currTimeState = loadedAnimSystem->getTime();
        loadedAnimSystem->setTime(currTimeState + 3333u);

        loadedScene->flush();
        std::this_thread::sleep_for(std::chrono::seconds(30));

        loadedScene->unpublish();
        ramses.destroy(*loadedScene);
        framework.disconnect();
    }
    return 0;
}
