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

#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Effect.h"
#include "ramses/client/MeshNode.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/DataTypes.h"

namespace ramses::internal
{
    struct TriangleTestScene
    {
        TriangleTestScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
        {
            scene = client.createScene(sceneId, "simple triangle scene");
            camera = scene->createPerspectiveCamera();
            camera->setFrustum(20.0f, 1.0f, 0.1f, 100.0f);
            camera->setViewport(0, 0, 800, 800);
            camera->setTranslation({0.0f, 0.0f, 5.0f});
            renderPass = scene->createRenderPass();
            renderPass->setClearFlags(ramses::EClearFlag::None);
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

            const ramses::Effect* effect = scene->createEffect(effectDesc);
            appearance = scene->createAppearance(*effect);

            ramses::Geometry* geometry = scene->createGeometry(*effect);
            geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);

            meshNode = scene->createMeshNode("triangle mesh node");
            meshNode->setAppearance(*appearance);
            meshNode->setIndexCount(3);
            meshNode->setGeometry(*geometry);

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
        explicit RamsesTestSetup(EFeatureLevel featureLevel = EFeatureLevel_Latest)
        {
            ramses::RamsesFrameworkConfig frameworkConfig{ featureLevel };
            frameworkConfig.setLogLevel(ramses::ELogLevel::Off);
            m_ramsesFramework = std::make_unique<ramses::RamsesFramework>(frameworkConfig);
            m_ramsesClient = m_ramsesFramework->createClient("test client");
            assert(m_ramsesClient);
        }

        ramses::RamsesFramework& getFramework()
        {
            return *m_ramsesFramework;
        }

        ramses::RamsesClient& getClient()
        {
            return *m_ramsesClient;
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
