//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <array>
#include <string_view>

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    struct TriangleTestScene
    {
        TriangleTestScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
        {
            scene = client.createScene(sceneId, ramses::SceneConfig(), "simple triangle scene");
            camera = scene->createPerspectiveCamera();
            camera->setFrustum(20.0f, 1.0f, 0.1f, 100.0f);
            camera->setViewport(0, 0, 800, 800);
            camera->setTranslation({0.0f, 0.0f, 5.0f});
            renderPass = scene->createRenderPass();
            renderPass->setClearFlags(ramses::EClearFlags_None);
            renderPass->setCamera(*camera);
            renderGroup = scene->createRenderGroup();
            renderPass->addRenderGroup(*renderGroup);

            std::array<ramses::vec3f, 3u> vertexPositionsArray{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
            ramses::ArrayResource* vertexPositions = scene->createArrayResource(3u, vertexPositionsArray.data());

            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(R"(
                #version 100

                uniform highp mat4 mvpMatrix;

                attribute vec3 a_position;

                void main()
                {
                    gl_Position = mvpMatrix * vec4(a_position, 1.0);
                }
                )");
            effectDesc.setFragmentShader(R"(
                #version 100

                uniform highp float green;
                uniform highp float blue;

                void main(void)
                {
                    gl_FragColor = vec4(1.0, green, blue, 1.0);
                }
                )");

            effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

            const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
            appearance = scene->createAppearance(*effect);

            ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect);
            ramses::AttributeInput positionsInput;
            effect->findAttributeInput("a_position", positionsInput);
            geometry->setInputBuffer(positionsInput, *vertexPositions);

            meshNode = scene->createMeshNode("triangle mesh node");
            meshNode->setAppearance(*appearance);
            meshNode->setIndexCount(3);
            meshNode->setGeometryBinding(*geometry);

            renderGroup->addMeshNode(*meshNode);

            scene->flush();
            scene->publish();
        }

        ramses::Scene*             scene;
        ramses::PerspectiveCamera* camera;
        ramses::RenderPass*        renderPass;
        ramses::RenderGroup*       renderGroup;
        ramses::Appearance*        appearance;
        ramses::MeshNode*          meshNode;
    };


    class RamsesTestSetup
    {
    public:
        RamsesTestSetup()
        {
            ramses::RamsesFrameworkConfig frameworkConfig;
            frameworkConfig.setLogLevel(ramses::ELogLevel::Off);
            m_ramsesFramework = std::make_unique<ramses::RamsesFramework>(frameworkConfig);
            m_ramsesClient = m_ramsesFramework->createClient("test client");
        }

        ramses::Scene* createScene(ramses::sceneId_t sceneId = ramses::sceneId_t(1))
        {
            return m_ramsesClient->createScene(sceneId);
        }

        void destroyScene(ramses::Scene& scene)
        {
            m_ramsesClient->destroy(scene);
        }

        ramses::Scene& loadSceneFromFile(std::string_view fileName)
        {
            auto scene = m_ramsesClient->loadSceneFromFile(fileName);
            assert(scene);
            return *scene;
        }

        static ramses::Appearance& CreateTestAppearance(ramses::Scene& scene, std::string_view vertShader, std::string_view fragShader)
        {
            ramses::EffectDescription effectDesc;
            effectDesc.setUniformSemantic("u_DisplayBufferResolution", ramses::EEffectUniformSemantic::DisplayBufferResolution);
            effectDesc.setVertexShader(vertShader.data());
            effectDesc.setFragmentShader(fragShader.data());
            return *scene.createAppearance(*scene.createEffect(effectDesc), "test appearance");
        }

        static ramses::Appearance& CreateTrivialTestAppearance(ramses::Scene& scene)
        {
            std::string_view vertShader = R"(
                #version 100

                uniform highp float floatUniform;
                uniform highp mat4 jointMat[1];
                attribute vec3 a_position;

                void main()
                {
                    gl_Position = floatUniform * vec4(a_position, 1.0) * jointMat[0];
                })";

            std::string_view fragShader = R"(
                #version 100

                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                })";

            return CreateTestAppearance(scene, vertShader, fragShader);
        }

        TriangleTestScene createTriangleTestScene(ramses::sceneId_t sceneId = ramses::sceneId_t(1))
        {
            return TriangleTestScene(*m_ramsesClient, sceneId);
        }

    private:
        std::unique_ptr<ramses::RamsesFramework> m_ramsesFramework;
        ramses::RamsesClient* m_ramsesClient;
    };
}
