//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <thread>

#include "ramses-client.h"

int main(int argc, char* argv[])
{
    ramses::RamsesFramework framework(argc, argv);

    ramses::RamsesClient client("workshop example client", framework);

    framework.connect();

    ramses::Scene* exampleScene = client.createScene(123);

    ramses::RenderPass* renderPass = exampleScene->createRenderPass();

    ramses::OrthographicCamera* camera = exampleScene->createOrthographicCamera();
    camera->setViewport(0, 0, 1280, 480);
    camera->setFrustum(0.0f, 1280.0f, 0.0f, 480.0f, 1.0f, 10.0f);
    renderPass->setCamera(*camera);

    ramses::RenderGroup* renderGroup = exampleScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::MeshNode* mesh = exampleScene->createMeshNode();
    renderGroup->addMeshNode(*mesh);

    float vertexPositionsArray[] = {
        300.f, 100.f, -1.f,
        600.f, 100.f, -1.f,
        450.f, 400.f, -1.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);

    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShaderFromFile("res/helloWorld-color.vert");
    effectDescription.setFragmentShaderFromFile("res/helloWorld-color.frag");
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effect = client.createEffect(effectDescription);

    ramses::AttributeInput positionInput;
    effect->findAttributeInput("a_position", positionInput);

    ramses::GeometryBinding* geometryBinding = exampleScene->createGeometryBinding(*effect);
    geometryBinding->setInputBuffer(positionInput, *vertexPositions);
    mesh->setGeometryBinding(*geometryBinding);
    mesh->setIndexCount(3);

    ramses::Appearance* appearance = exampleScene->createAppearance(*effect);
    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.0f, 1.0f);
    mesh->setAppearance(*appearance);

    exampleScene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);
    exampleScene->publish();

    std::this_thread::sleep_for(std::chrono::seconds(30));

    exampleScene->unpublish();
    client.destroy(*exampleScene);
    framework.disconnect();
}
