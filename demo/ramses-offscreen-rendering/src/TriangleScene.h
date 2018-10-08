//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMO_TRIANGLESCENE_H
#define RAMSES_DEMO_TRIANGLESCENE_H

#include "ramses-client.h"

struct TriangleColor
{
    float red;
    float green;
    float blue;
};

struct TriangleScene
{
    ramses::Scene* scene;
    ramses::RotateNode* rotateNode;
};

TriangleScene CreateTriangleScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, TriangleColor color, ramses::EScenePublicationMode publicationMode)
{
    ramses::Scene* scene = client.createScene(sceneId, ramses::SceneConfig(), "triangle scene");

    // Set up render pass and camera
    ramses::Camera* camera = scene->createRemoteCamera("my camera");
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // Create effect and appearance
    const char* vertexShader =
        "#version 100                                           \n"
        "                                                       \n"
        "uniform highp mat4 mvpMatrix;                          \n"
        "                                                       \n"
        "attribute vec3 a_position;                             \n"
        "                                                       \n"
        "void main()                                            \n"
        "{                                                      \n"
        "    gl_Position = mvpMatrix * vec4(a_position, 1.0);   \n"
        "}                                                      \n";

    const char* fragmentShader =
        "#version 100               \n"
        "                           \n"
        "uniform highp vec4 color;  \n"
        "void main(void)            \n"
        "{                          \n"
        "    gl_FragColor = color;  \n"
        "}                          \n";

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShader(vertexShader);
    effectDesc.setFragmentShader(fragmentShader);
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");
    appearance->setCullingMode(ramses::ECullMode_Disabled);
    appearance->setDepthFunction(ramses::EDepthFunc_Always);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, color.red, color.green, color.blue, 1.0f);

    // create geometry
    float vertexPositionsArray[] = { -0.25f, -0.125f, 0.f, 0.25f, -0.125f, 0.f, 0.f, 0.125f, 0.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");
    geometry->setIndices(*indices);

    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // Put a mesh node behind the camera
    ramses::TranslateNode* rootTranslation = scene->createTranslateNode("rootNode");
    rootTranslation->setTranslation(0.0f, 0.0f, -1.0f);
    ramses::RotateNode* rotationNode = scene->createRotateNode("rotationNode");
    rotationNode->setParent(*rootTranslation);

    ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    meshNode->setParent(*rotationNode);
    renderGroup->addMeshNode(*meshNode);

    // Finalize scene state and publish
    scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);
    scene->publish(publicationMode);

    return {scene, rotationNode};
}

#endif
