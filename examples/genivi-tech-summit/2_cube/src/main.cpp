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

using Vertex = std::array<float, 3>;

void addTexturedQuad(const std::array<Vertex, 4>& vertices,
    ramses::RamsesClient& ramsesClient, ramses::Scene& scene, const char* /*textureFile*/, ramses::RenderGroup& renderGroup,
    ramses::GroupNode& groupNode, ramses::Effect& effect)
{
    std::vector<float> vertexData;
    vertexData.insert(vertexData.end(), vertices[0].begin(), vertices[0].end());
    vertexData.insert(vertexData.end(), vertices[1].begin(), vertices[1].end());
    vertexData.insert(vertexData.end(), vertices[2].begin(), vertices[2].end());
    vertexData.insert(vertexData.end(), vertices[3].begin(), vertices[3].end());

    const ramses::Vector3fArray* vertexArray = ramsesClient.createConstVector3fArray(4, vertexData.data());

    ramses::AttributeInput positionAttributeInput;
    effect.findAttributeInput("a_position", positionAttributeInput);

    ramses::Appearance* appearance = scene.createAppearance(effect);
    appearance->setDrawMode(ramses::EDrawMode_TriangleStrip);

    ramses::GeometryBinding* geometrybinding = scene.createGeometryBinding(effect);
    geometrybinding->setInputBuffer(positionAttributeInput, *vertexArray);

    // [add code here]

    ramses::MeshNode* mesh = scene.createMeshNode();
    mesh->setGeometryBinding(*geometrybinding);
    mesh->setAppearance(*appearance);
    mesh->setIndexCount(4);

    mesh->setParent(groupNode);
    renderGroup.addMeshNode(*mesh);
}

int main(int argc, char* argv[])
{
    int instance = 0;
    if (argc > 1)
    {
        instance = atoi(argv[1]);
    }

    ramses::RamsesFramework framework(argc, argv);

    ramses::RamsesClient client("workshop example client", framework);

    framework.connect();

    ramses::Scene* scene = client.createScene(instance);

    ramses::RenderPass* renderPass = scene->createRenderPass();

    ramses::Camera* camera = scene->createRemoteCamera();
    renderPass->setCamera(*camera);
    camera->translate(0, 0, 5);

    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShaderFromFile("res/cube-texturing.vert");
    effectDescription.setFragmentShaderFromFile("res/cube-texturing.frag");
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effect = client.createEffect(effectDescription);

    // [add code here]

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
    const Vertex B = {  0.5f, -0.5f,  0.5f };
    const Vertex C = { -0.5f,  0.5f,  0.5f };
    const Vertex D = {  0.5f,  0.5f,  0.5f };
    const Vertex E = {  0.5f, -0.5f, -0.5f };
    const Vertex F = { -0.5f, -0.5f, -0.5f };
    const Vertex G = {  0.5f,  0.5f, -0.5f };
    const Vertex H = { -0.5f,  0.5f, -0.5f };

    addTexturedQuad({ A, B, C, D }, client, *scene, "res/cube-face-1.png", *renderGroup, *groupNode, *effect);
    addTexturedQuad({ B, E, D, G }, client, *scene, "res/cube-face-2.png", *renderGroup, *groupNode, *effect);
    addTexturedQuad({ C, D, H, G }, client, *scene, "res/cube-face-3.png", *renderGroup, *groupNode, *effect);
    addTexturedQuad({ E, F, G, H }, client, *scene, "res/cube-face-1.png", *renderGroup, *groupNode, *effect);
    addTexturedQuad({ F, A, H, C }, client, *scene, "res/cube-face-2.png", *renderGroup, *groupNode, *effect);
    addTexturedQuad({ F, E, A, B }, client, *scene, "res/cube-face-3.png", *renderGroup, *groupNode, *effect);

    groupNode->rotate(-20.0f, 30.0f, 0.0f);

    if (instance == 1)
    {
        groupNode->translate(-1, 0, 0);
    }
    else if (instance == 2)
    {
        groupNode->translate(1, 0, 0);
    }

    scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);
    scene->publish(ramses::EScenePublicationMode_LocalAndRemote);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    scene->unpublish();
    client.destroy(*scene);
    framework.disconnect();

    return 0;
}
