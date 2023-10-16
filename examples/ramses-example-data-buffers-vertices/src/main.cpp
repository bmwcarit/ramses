//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"

#include <thread>
#include <chrono>
#include <cmath>
#include <vector>

/**
 * @example ramses-example-data-buffers-vertices/src/main.cpp
 * @brief Basic Data Buffer Example
 */

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-data-buffers-vertices"));
    framework.connect();

    // create a scene for distributing content
    const ramses::SceneConfig sceneConfig(ramses::sceneId_t{123}, ramses::EScenePublicationMode::LocalAndRemote);
    ramses::Scene* scene = ramses.createScene(sceneConfig, "triangle scene");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 35.0f});
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // create an appearance for triangles
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-data-buffers-vertices.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-data-buffers-vertices.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = scene->createEffect(effectDesc, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance data buffer");
    ramses::Geometry* geometry = scene->createGeometry(*effect, "triangle geometry data buffer");

    // get input data of appearance and set color values
    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(colorInput.has_value());
    appearance->setInputValue(*colorInput, ramses::vec4f{ 0.0f, 1.0f, 1.0f, 1.0f });

    // the raw data for vertices and indices
    const std::vector<ramses::vec3f> vertexPositions = {
        ramses::vec3f{ -1.f,  4.f, -1.f }, ramses::vec3f{ 0.f,  4.f, -1.f }, ramses::vec3f{ 1.f,  4.f, -1.f },
        ramses::vec3f{ -2.f,  2.f, -1.f },                                   ramses::vec3f{ 2.f,  2.f, -1.f },
        ramses::vec3f{ -4.f,  1.f, -1.f },                                   ramses::vec3f{ 4.f,  1.f, -1.f },
        ramses::vec3f{ -4.f,  0.f, -1.f },                                   ramses::vec3f{ 4.f,  0.f, -1.f },
        ramses::vec3f{ -4.f, -1.f, -1.f },                                   ramses::vec3f{ 4.f, -1.f, -1.f },
        ramses::vec3f{ -2.f, -2.f, -1.f },                                   ramses::vec3f{ 2.f, -2.f, -1.f },
        ramses::vec3f{ -1.f, -4.f, -1.f }, ramses::vec3f{ 0.f, -4.f, -1.f }, ramses::vec3f{ 1.f, -4.f, -1.f } };


    const std::vector<uint16_t> indexData = {
        3,   1,  0,     4,  2,  1,     4,  1,  3,
        8,   6,  4,    12, 10,  8,    12,  8,  4,
        14, 15, 12,    11, 13, 14,    11, 14, 12,
        7,   9, 11,     3,  5,  7,     3,  7, 11,
        4,   3, 11,    11, 12,  4
    };

    /// [Data Buffer Example Setup]
    /// Creating a shape with #ramses::ArrayBuffer vertices and constantly changing them afterwards
    //
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Create the ArrayBuffers
    // The data buffers need more information about size
    const auto NumVertices = uint32_t(vertexPositions.size());
    const auto NumIndices = uint32_t(indexData.size());

    // then create the buffers via the _scene_
    ramses::ArrayBuffer* vertices = scene->createArrayBuffer(ramses::EDataType::Vector3F, NumVertices, "some varying vertices");
    ramses::ArrayBuffer* indices = scene->createArrayBuffer(ramses::EDataType::UInt16, NumIndices, "some varying indices");

    // finally set/update the data
    vertices->updateData(0u, NumVertices, vertexPositions.data());
    indices->updateData(0u, NumIndices, indexData.data());

    /// [Data Buffer Example Setup]

    // applying the vertex/index data to the geometry binding is the same for both
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertices);
    geometry->setIndices(*indices);


    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node data buffers");

    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);

    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish(ramses::EScenePublicationMode::LocalAndRemote);

    // application logic
    auto translateVertex = [&vertexPositions](ramses::vec3f& updatedValues, uint32_t index, uint64_t timeStamp, float scaleFactor){
        const auto currentFactor = std::sin(0.005f * static_cast<float>(timeStamp));
        const float    xValue     = vertexPositions[index][0];
        const float    yValue     = vertexPositions[index][1];
        const float    zValue     = vertexPositions[index][2];

        updatedValues[0] = xValue + scaleFactor * xValue * currentFactor;
        updatedValues[1] = yValue + scaleFactor * yValue * currentFactor;
        updatedValues[2] = zValue;
    };

    /// [Data Buffer Example Loop]
    const std::vector<uint32_t> ridges  = { 1, 7,  8, 14 };
    const std::vector<uint32_t> notches = { 3, 4, 11, 12 };

    ramses::vec3f updatedPosition;
    for (uint64_t timeStamp = 0u; timeStamp < 10000u; timeStamp += 20u)
    {
        for(auto i: ridges)
        {
            translateVertex(updatedPosition, i, timeStamp, 0.25f);
            vertices->updateData(i, 1, &updatedPosition);
        }

        for(auto i: notches)
        {
            translateVertex(updatedPosition, i, timeStamp, -0.4f);
            vertices->updateData(i, 1, &updatedPosition);
        }

        // signal the scene it is in a state that can be rendered
        scene->flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    /// [Data Buffer Example Loop]

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
