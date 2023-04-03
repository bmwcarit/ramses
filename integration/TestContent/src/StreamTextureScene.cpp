//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/StreamTextureScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include <array>

namespace ramses_internal
{
    StreamTextureScene::StreamTextureScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_effect(nullptr)
    {
        m_effect = getTestEffect("ramses-test-client-textured");
        m_root = m_scene.createNode();

        const ramses::vec3f A{ -0.5f,  0.5f, -0.5f };
        const ramses::vec3f B{ -0.5f, -0.5f, -0.5f };
        const ramses::vec3f C{  0.5f, -0.5f, -0.5f };
        const ramses::vec3f D{  0.5f,  0.5f, -0.5f };
        const ramses::vec3f E{ -0.5f,  0.5f,  0.5f };
        const ramses::vec3f F{ -0.5f, -0.5f,  0.5f };
        const ramses::vec3f G{  0.5f, -0.5f,  0.5f };
        const ramses::vec3f H{  0.5f,  0.5f,  0.5f };

        using QuadVerts = std::array<ramses::vec3f, 4>;
        const QuadVerts verts1{{A, B, C, D}};
        const QuadVerts verts2{{C, G, H, D}};
        const QuadVerts verts3{{H, E, A, D}};
        const QuadVerts verts4{{F, G, C, B}};
        const QuadVerts verts5{{F, B, A, E}};
        const QuadVerts verts6{{F, E, H, G}};

        switch (state)
        {
            case MULTI_SOURCE_SAME_FALLBACK_TEXTURE:
            {
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts1, TextureConsumers[0]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2, TextureConsumers[1]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts3, TextureConsumers[2]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts4, TextureConsumers[3]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts5, TextureConsumers[4]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts6, TextureConsumers[5]);
                break;
            }
            case SAME_SOURCE_MULTI_FALLBACK:
            {
                addPngQuad("res/ramses-test-client-embedded-compositing-1.png", verts1, TextureConsumers[0]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2, TextureConsumers[1]);
                addPngQuad("res/ramses-test-client-embedded-compositing-3.png", verts3, TextureConsumers[2]);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4, TextureConsumers[3]);
                addPngQuad("res/ramses-test-client-embedded-compositing-5.png", verts5, TextureConsumers[4]);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6, TextureConsumers[5]);
                break;
            }
            case MULTI_SOURCE_MULTI_FALLBACK:
            {
                //Create two separate half cubes which are visible from the camera position
                //Each half cube is formed from 3 surfaces that uses vertex arrays 2, 4 and 6
                ramses::Node* translateLeftNode = m_scene.createNode("translate left node");
                translateLeftNode->translate(-1.0f, 0.0f, 0.0f);
                m_root->addChild(*translateLeftNode);

                ramses::Node* translateRightNode = m_scene.createNode("translate right node");
                translateRightNode->translate(1.0f, 0.0f, 0.0f);
                m_root->addChild(*translateRightNode);

                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2, TextureConsumers[0], translateLeftNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4, TextureConsumers[1], translateLeftNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6, TextureConsumers[2], translateLeftNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2, TextureConsumers[3], translateRightNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4, TextureConsumers[4], translateRightNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6, TextureConsumers[5], translateRightNode);
                break;

            }
            case INITIAL_STATE:
            {
                addPngQuad("res/ramses-test-client-embedded-compositing-1.png", verts1, TextureConsumers[0]);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2, TextureConsumers[1]);
                addPngQuad("res/ramses-test-client-embedded-compositing-3.png", verts3, TextureConsumers[2]);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4, TextureConsumers[3]);
                addPngQuad("res/ramses-test-client-embedded-compositing-5.png", verts5, TextureConsumers[4]);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6, TextureConsumers[5]);
                break;
            }
        }

        m_root->setRotation(-35.264385f, -45.000427f, 0.000427f, ramses::ERotationConvention::Euler_XYZ); // rotate the cube onto one corner
    }

    void StreamTextureScene::addPngQuad(const char* pngFilePath, const std::array<ramses::vec3f, 4u>& vertexPositionsArray, ramses::dataConsumerId_t consumerId, ramses::Node* parentNode)
    {
        const ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng(pngFilePath, m_scene);
        const ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest,
            ramses::ETextureSamplingMethod_Nearest,
            *texture);

        m_scene.createTextureConsumer(*sampler, consumerId);

        ramses::Appearance* appearance = m_scene.createAppearance(*m_effect, "triangle appearance");

        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray
        {
            ramses::vec2f{ 0.f, 1.f }, //A   A-----D
            ramses::vec2f{ 0.f, 0.f }, //B   |     |
            ramses::vec2f{ 1.f, 0.f }, //C   |     |
            ramses::vec2f{ 1.f, 1.f }  //D   B-----C
        };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        const uint16_t indicesArray[] =
        {
            0, 2, 1, //ACB
            0, 3, 2  //ADC
        };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6u, indicesArray);

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        m_effect->findAttributeInput("a_position", positionsInput);
        m_effect->findAttributeInput("a_texcoord", texCoordsInput);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*m_effect, "triangle geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *textureCoords);

        ramses::UniformInput textureInput;
        m_effect->findUniformInput("u_texture", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("textured triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);

        if (parentNode == nullptr)
            parentNode = m_root;
        parentNode->addChild(*meshNode);

        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);
    }
}
