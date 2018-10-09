//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <thread>
#include <array>
#include <vector>

#include "ramses-client.h"
#include "ramses-utils.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/ResourceFileDescription.h"

using Vertex = std::array<float, 3>;

void addTexturedQuad(const std::array<Vertex, 4>& vertices,
    ramses::RamsesClient& ramsesClient, ramses::Scene& scene, const char* textureFile, ramses::RenderGroup& renderGroup,
    ramses::GroupNode& groupNode, ramses::Effect& effect, const ramses::Vector2fArray& textureCoordsArray, ramses::ResourceFileDescription& resources)
{
    std::vector<float> vertexData;
    vertexData.insert(vertexData.end(), vertices[0].begin(), vertices[0].end());
    vertexData.insert(vertexData.end(), vertices[1].begin(), vertices[1].end());
    vertexData.insert(vertexData.end(), vertices[2].begin(), vertices[2].end());
    vertexData.insert(vertexData.end(), vertices[3].begin(), vertices[3].end());

    const ramses::Vector3fArray* vertexArray = ramsesClient.createConstVector3fArray(4, vertexData.data());
    ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng(textureFile, ramsesClient);
    ramses::TextureSampler* sampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat,
        ramses::ETextureSamplingMethod_Bilinear, *texture);

    ramses::AttributeInput positionAttributeInput;
    effect.findAttributeInput("a_position", positionAttributeInput);

    ramses::AttributeInput textCoordInput;
    effect.findAttributeInput("a_texcoord", textCoordInput);

    ramses::UniformInput textureSamplerInput;
    effect.findUniformInput("textureSampler", textureSamplerInput);

    ramses::Appearance* appearance = scene.createAppearance(effect);
    appearance->setDrawMode(ramses::EDrawMode_TriangleStrip);
    appearance->setInputTexture(textureSamplerInput, *sampler);

    ramses::GeometryBinding* geometrybinding = scene.createGeometryBinding(effect);
    geometrybinding->setInputBuffer(positionAttributeInput, *vertexArray);
    geometrybinding->setInputBuffer(textCoordInput, textureCoordsArray);

    ramses::MeshNode* mesh = scene.createMeshNode();
    mesh->setGeometryBinding(*geometrybinding);
    mesh->setAppearance(*appearance);
    mesh->setIndexCount(4);

    mesh->setParent(groupNode);
    renderGroup.addMeshNode(*mesh);

    resources.add(vertexArray);
    resources.add(texture);
}

void createAndSaveScene()
{
    ramses::RamsesFramework framework;
    ramses::RamsesClient client("workshop example client", framework);

    ramses::Scene* scene = client.createScene(666u);

    ramses::RenderPass* renderPass = scene->createRenderPass();

    ramses::Camera* camera = scene->createRemoteCamera();
    renderPass->setCamera(*camera);
    camera->translate(0, 0, 5);

    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShaderFromFile("res/remoteAnimation-texturing.vert");
    effectDescription.setFragmentShaderFromFile("res/remoteAnimation-texturing.frag");
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effect = client.createEffect(effectDescription);

    const float textureCoordsData[] = {
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };
    const ramses::Vector2fArray* textureCoordsArray = client.createConstVector2fArray(4, textureCoordsData);

    ramses::GroupNode* groupNode = scene->createGroupNode();

    /* prepare triangle geometry for the cube faces
    *
    *    H-------G
    *   /|      /|
    *  C-------D |     2-top-3                          +y|   /-z
    *  | |     | |     | \   | per side                   |  /
    *  | F-----|-E     |   \ | vertex strip order         | /
    *  |/      |/      0-bot-1                            |/_____ +x
    *  A-------B
    */

    const Vertex A = { -0.5f, -0.5f,  0.5f };
    const Vertex B = { 0.5f, -0.5f,  0.5f };
    const Vertex C = { -0.5f,  0.5f,  0.5f };
    const Vertex D = { 0.5f,  0.5f,  0.5f };
    const Vertex E = { 0.5f, -0.5f, -0.5f };
    const Vertex F = { -0.5f, -0.5f, -0.5f };
    const Vertex G = { 0.5f,  0.5f, -0.5f };
    const Vertex H = { -0.5f,  0.5f, -0.5f };

    ramses::ResourceFileDescription resources("resources.res");
    resources.add(effect);
    resources.add(textureCoordsArray);

    addTexturedQuad({ A, B, C, D }, client, *scene, "res/remoteAnimation-face-1.png", *renderGroup, *groupNode, *effect, *textureCoordsArray, resources);
    addTexturedQuad({ B, E, D, G }, client, *scene, "res/remoteAnimation-face-2.png", *renderGroup, *groupNode, *effect, *textureCoordsArray, resources);
    addTexturedQuad({ C, D, H, G }, client, *scene, "res/remoteAnimation-face-3.png", *renderGroup, *groupNode, *effect, *textureCoordsArray, resources);
    addTexturedQuad({ E, F, G, H }, client, *scene, "res/remoteAnimation-face-1.png", *renderGroup, *groupNode, *effect, *textureCoordsArray, resources);
    addTexturedQuad({ F, A, H, C }, client, *scene, "res/remoteAnimation-face-2.png", *renderGroup, *groupNode, *effect, *textureCoordsArray, resources);
    addTexturedQuad({ F, E, A, B }, client, *scene, "res/remoteAnimation-face-3.png", *renderGroup, *groupNode, *effect, *textureCoordsArray, resources);

    groupNode->rotate(-20.0f, 30.0f, 0.0f);
    groupNode->translate(-1, 0, 0);

    ramses::AnimationSystem* animationSystem = scene->createRealTimeAnimationSystem();

    ramses::SplineLinearVector3f* splineRotation = animationSystem->createSplineLinearVector3f();
    splineRotation->setKey(0, 0.f, 0.f, 0.f);
    splineRotation->setKey(57600, 1080.f, 2160.f, 360.f); //x-axis: 3 rotations, y-axis: 6 rotations, z-axis: 1 rotation

    ramses::AnimatedProperty* animPropertyRototation = animationSystem->createAnimatedProperty(*groupNode, ramses::EAnimatedProperty_Rotation,
        ramses::EAnimatedPropertyComponent_All);
    ramses::Animation* animation = animationSystem->createAnimation(*animPropertyRototation, *splineRotation);

    ramses::AnimationSequence* sequence = animationSystem->createAnimationSequence();
    sequence->addAnimation(*animation);
    sequence->setAnimationLooping(*animation);
    auto now = std::chrono::system_clock::now();
    sequence->startAt(std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count());

    scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);

    ramses::ResourceFileDescriptionSet resourcesSet;
    resourcesSet.add(resources);
    client.saveSceneToFile(*scene, "scene.ramses", resourcesSet, false);
}

int main(int argc, char* argv[])
{
    createAndSaveScene();

    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient client("workshop example client", framework);

    framework.connect();

    //[add code here] : replace scene with scene from file
    ramses::Scene* scene = client.createScene(1);

    scene->unpublish();
    client.destroy(*scene);
    framework.disconnect();
}
