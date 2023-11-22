//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/FileLoadingScene.h"
#include "ramses/client/ramses-utils.h"

#include "ramses/client/Scene.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/UniformInput.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "internal/Core/Utils/File.h"

namespace ramses::internal
{
    FileLoadingScene::FileLoadingScene(ramses::RamsesClient& clientForLoading, uint32_t state, ramses::sceneId_t sceneId, const glm::vec3& cameraPosition, const std::string& folder, const ramses::RamsesFrameworkConfig& config, uint32_t vpWidth, uint32_t vpHeight)
        : m_viewportWidth(vpWidth)
        , m_viewportHeight(vpHeight)
    {
        switch (state)
        {
        case CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT:
        {
            ramses::RamsesFramework separateFramework(config);
            ramses::RamsesClient& separateClient(*separateFramework.createClient("ramses-test-client-fileLoadingScene-createFiles"));
            createFiles(separateClient, sceneId, cameraPosition, folder);
            loadFromFiles(clientForLoading, folder);
            CleanupFiles(folder);
            break;
        }
        case CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT:
        {
            createFiles(clientForLoading, sceneId, cameraPosition, folder);
            loadFromFiles(clientForLoading, folder);
            CleanupFiles(folder);
            break;
        }
        default:
            assert(false && "Invalid state for FileLoadingScene");
            break;
        }
    }

    void FileLoadingScene::createFiles(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const glm::vec3& cameraPosition, const std::string& folder)
    {
        ramses::Scene* scene = client.createScene(SceneConfig(sceneId));

        ramses::Node* cameraTranslation = scene->createNode("cameraPosition");
        cameraTranslation->setTranslation({cameraPosition.x, cameraPosition.y, cameraPosition.z});
        auto camera = scene->createPerspectiveCamera("fileLoading camera");
        camera->setViewport(0, 0, m_viewportWidth, m_viewportHeight);
        camera->setFrustum(19.f, float(m_viewportWidth) / m_viewportHeight, 0.1f, 1500.f);
        camera->setParent(*cameraTranslation);
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlag::None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup("render group");
        renderPass->addRenderGroup(*renderGroup);

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{ ramses::vec3f{-0.5f, -0.5f, -1.f}, ramses::vec3f{0.5f, -0.5f, -1.f}, ramses::vec3f{-0.5f, 0.5f, -1.f}, ramses::vec3f{0.5f, 0.5f, -1.f} };
        ramses::ArrayResource* vertexPositions = scene->createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
        ramses::ArrayResource* textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        ramses::ArrayResource* indices = scene->createArrayResource(6u, indicesArray);
        ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-file-loading-texture.png", *scene);
        assert(texture != nullptr);

        ramses::TextureSampler* sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Nearest,
            ramses::ETextureSamplingMethod::Nearest,
            *texture);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-file-loading-texturing.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-file-loading-texturing.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* effectTex = scene->createEffect(effectDesc, "glsl shader");

        ramses::Appearance* appearance = scene->createAppearance(*effectTex, "triangle appearance");
        ramses::Geometry* geometry = scene->createGeometry(*effectTex, "triangle geometry");

        geometry->setIndices(*indices);
        geometry->setInputBuffer(*effectTex->findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*effectTex->findAttributeInput("a_texcoord"), *textureCoords);
        appearance->setInputTexture(*effectTex->findUniformInput("textureSampler"), *sampler);

        ramses::Node* scaleNode = scene->createNode("scale node");

        ramses::MeshNode* meshNode = scene->createMeshNode("textured triangle mesh node");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);
        renderGroup->addMeshNode(*meshNode);

        scaleNode->addChild(*meshNode);

        AddTriangles(*scene, *renderGroup);

        [[maybe_unused]] auto status = scene->saveToFile(folder + "/tempfile.ramses", {});

        client.destroy(*scene);
    }

    void FileLoadingScene::AddTriangles(ramses::Scene& scene, ramses::RenderGroup& renderGroup)
    {
        // prepare triangle geometry: vertex position array and index array
        const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-0.3f, 0.f, -0.3f}, ramses::vec3f{0.3f, 0.f, -0.3f}, ramses::vec3f{0.f, 0.3f, -0.3f} };
        ramses::ArrayResource* vertexPositions = scene.createArrayResource(3u, vertexPositionsData.data());

        uint16_t indexData[] = { 0, 1, 2 };
        ramses::ArrayResource* indices = scene.createArrayResource(3u, indexData);

        // create an appearance for red triangle
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-file-loading-basic.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-file-loading-red.frag");
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* effect = scene.createEffect(effectDesc, "glsl shader anim");
        ramses::Appearance* appearance = scene.createAppearance(*effect, "triangle appearance anim");

        // set vertex positions directly in geometry
        ramses::Geometry* geometry = scene.createGeometry(*effect, "triangle geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);

        // create a mesh nodes to define the triangles with chosen appearance
        ramses::MeshNode* meshNode1 = scene.createMeshNode("red triangle mesh node1");
        meshNode1->setAppearance(*appearance);
        meshNode1->setGeometry(*geometry);
        ramses::MeshNode* meshNode2 = scene.createMeshNode("red triangle mesh node2");
        meshNode2->setAppearance(*appearance);
        meshNode2->setGeometry(*geometry);
        ramses::MeshNode* meshNode3 = scene.createMeshNode("red triangle mesh node3");
        meshNode3->setAppearance(*appearance);
        meshNode3->setGeometry(*geometry);

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

        transNode1->setTranslation({-0.133f, 0.f, 0.f});
        transNode2->setTranslation({0.133f, 0.f, 0.f});
        transNode3->setTranslation({0.f, -0.133f, 0.f});
    }

    void FileLoadingScene::loadFromFiles(ramses::RamsesClient& client, const std::string& folder)
    {
        ramses::Scene* loadedScene = client.loadSceneFromFile(folder + "/tempfile.ramses", SceneConfig({}, EScenePublicationMode::LocalAndRemote));

        // make changes to loaded scene
        auto& loadedScaleNode = *loadedScene->findObject<ramses::Node>("scale node");
        loadedScaleNode.setScaling({2, 2, 2});
        loadedScene->flush();

        loadedScene->flush();
        m_createdScene = loadedScene;
    }

    ramses::Scene* FileLoadingScene::getCreatedScene()
    {
        return m_createdScene;
    }

    void FileLoadingScene::CleanupFiles(const std::string& folder)
    {
        for (const auto& name : {"/texture.ramres", "/triangle.ramres", "/tempfile.ramses"})
        {
            File file(folder + name);
            if (file.exists())
                file.remove();
        }
    }
}
