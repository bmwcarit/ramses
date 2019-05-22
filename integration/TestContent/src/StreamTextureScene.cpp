//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/StreamTextureScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/StreamTexture.h"
#include <array>

namespace ramses_internal
{
    StreamTextureScene::StreamTextureScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_effect(0)
    {
        m_effect = getTestEffect("ramses-test-client-textured");
        m_root = m_scene.createNode();

        using Vec3 = std::array<float, 3>;
        const Vec3 A{{ -0.5f,  0.5f, -0.5f }};
        const Vec3 B{{ -0.5f, -0.5f, -0.5f }};
        const Vec3 C{{  0.5f, -0.5f, -0.5f }};
        const Vec3 D{{  0.5f,  0.5f, -0.5f }};
        const Vec3 E{{ -0.5f,  0.5f,  0.5f }};
        const Vec3 F{{ -0.5f, -0.5f,  0.5f }};
        const Vec3 G{{  0.5f, -0.5f,  0.5f }};
        const Vec3 H{{  0.5f,  0.5f,  0.5f }};

        using QuadVerts = std::array<Vec3, 4>;
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
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts1.front().data(), ramses::streamSource_t(1));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts3.front().data(), ramses::streamSource_t(3));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts4.front().data(), ramses::streamSource_t(4));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts5.front().data(), ramses::streamSource_t(5));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts6.front().data(), ramses::streamSource_t(6));
                break;
            }
            case SAME_SOURCE_MULTI_FALLBACK:
            {
                addPngQuad("res/ramses-test-client-embedded-compositing-1.png", verts1.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-3.png", verts3.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-5.png", verts5.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6.front().data(), ramses::streamSource_t(2));
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

                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2.front().data(), ramses::streamSource_t(2), translateLeftNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4.front().data(), ramses::streamSource_t(2), translateLeftNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6.front().data(), ramses::streamSource_t(4), translateLeftNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2.front().data(), ramses::streamSource_t(4), translateRightNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4.front().data(), ramses::streamSource_t(2), translateRightNode);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6.front().data(), ramses::streamSource_t(6), translateRightNode);
                break;

            }
            case FORCE_FALLBACK_ON_SOME_TEXTURES:
            {
                addPngQuad("res/ramses-test-client-embedded-compositing-1.png", verts1.front().data(), ramses::streamSource_t(2), NULL, "stream1", true);
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2.front().data(), ramses::streamSource_t(2), NULL, "stream2", true);
                addPngQuad("res/ramses-test-client-embedded-compositing-3.png", verts3.front().data(), ramses::streamSource_t(2), NULL, "stream3", true);
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4.front().data(), ramses::streamSource_t(2), NULL, "stream4", false);
                addPngQuad("res/ramses-test-client-embedded-compositing-5.png", verts5.front().data(), ramses::streamSource_t(2), NULL, "stream5", true);
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6.front().data(), ramses::streamSource_t(2), NULL, "stream6", false);
                break;
            }
            case INITIAL_STATE:
            {
                addPngQuad("res/ramses-test-client-embedded-compositing-1.png", verts1.front().data(), ramses::streamSource_t(1));
                addPngQuad("res/ramses-test-client-embedded-compositing-2.png", verts2.front().data(), ramses::streamSource_t(2));
                addPngQuad("res/ramses-test-client-embedded-compositing-3.png", verts3.front().data(), ramses::streamSource_t(3));
                addPngQuad("res/ramses-test-client-embedded-compositing-4.png", verts4.front().data(), ramses::streamSource_t(4));
                addPngQuad("res/ramses-test-client-embedded-compositing-5.png", verts5.front().data(), ramses::streamSource_t(5));
                addPngQuad("res/ramses-test-client-embedded-compositing-6.png", verts6.front().data(), ramses::streamSource_t(6));
                break;
            }
        }

        m_root->setRotation(35.264385f, 45.000427f, -0.000427f); // rotate the cube onto one corner
    }

    void StreamTextureScene::addPngQuad(const char* pngFilePath, const float* vertexPositionsArray, ramses::streamSource_t surfaceId, ramses::Node* parentNode, const char* streamTextureName, bool forcefallback)
    {
        const ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng(pngFilePath, m_client);
        ramses::StreamTexture* streamTexture = m_scene.createStreamTexture(*texture, surfaceId, streamTextureName);
        streamTexture->forceFallbackImage(forcefallback);
        const ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest,
            ramses::ETextureSamplingMethod_Nearest,
            *streamTexture);

        ramses::Appearance* appearance = m_scene.createAppearance(*m_effect, "triangle appearance");

        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        const float textureCoordsArray[] =
        {
            0.f, 1.f, //A   A-----D
            0.f, 0.f, //B   |     |
            1.f, 0.f, //C   |     |
            1.f, 1.f  //D   B-----C
        };
        const ramses::Vector2fArray* textureCoords = m_client.createConstVector2fArray(4, textureCoordsArray);

        const uint16_t indicesArray[] =
        {
            0, 2, 1, //ACB
            0, 3, 2  //ADC
        };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

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
