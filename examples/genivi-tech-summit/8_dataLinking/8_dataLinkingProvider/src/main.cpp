//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <thread>

#include "ramses-client.h"

const ramses::sceneId_t ProviderSceneId = 11;
const ramses::dataProviderId_t ProviderDataId(1);

int main(int argc, char* argv[])
{
    ramses::RamsesFramework framework(argc, argv);

    ramses::RamsesClient client("workshop example client", framework);

    framework.connect();

    ramses::Scene* exampleScene = client.createScene(ProviderSceneId);

    ramses::RenderPass* renderPass = exampleScene->createRenderPass();

    ramses::Camera* camera = exampleScene->createRemoteCamera();
    renderPass->setCamera(*camera);
    camera->translate(4.0f, -0.5, 10);

    ramses::RenderGroup* renderGroup = exampleScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    float vertexPositionsArray[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, -1.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);

    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShaderFromFile("res/dataProvider-color.vert");
    effectDescription.setFragmentShaderFromFile("res/dataProvider-color.frag");
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effect = client.createEffect(effectDescription);

    ramses::AttributeInput positionInput;
    effect->findAttributeInput("a_position", positionInput);

    ramses::GeometryBinding* geometryBinding = exampleScene->createGeometryBinding(*effect);
    geometryBinding->setInputBuffer(positionInput, *vertexPositions);

    ramses::Appearance* appearance = exampleScene->createAppearance(*effect);
    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.0f, 1.0f);

    ramses::MeshNode* mesh = exampleScene->createMeshNode();
    renderGroup->addMeshNode(*mesh);
    mesh->setGeometryBinding(*geometryBinding);
    mesh->setIndexCount(3);
    mesh->setAppearance(*appearance);

    // [add code here]

    exampleScene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);
    exampleScene->publish();

    float translation = 0;
    while(1)
    {
        mesh->setTranslation(translation, 0.f, 0.f);
        exampleScene->flush();

        translation+=0.01f;
        if (translation >= 8.0f)
        {
            translation = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    exampleScene->unpublish();
    client.destroy(*exampleScene);
    framework.disconnect();
}
