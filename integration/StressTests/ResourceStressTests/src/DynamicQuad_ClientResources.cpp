//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_ClientResources.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/TextureSampler.h"

#include "PlatformAbstraction/PlatformMath.h"
#include "TestRandom.h"
#include <memory>

namespace ramses_internal
{
    DynamicQuad_ClientResources::DynamicQuad_ClientResources(ramses::RamsesClient& client, ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(client, scene, screenspaceQuad)
    {
        m_renderGroup.addMeshNode(m_meshNode);

        m_quadResources = createRandomizedQuadResources();
        m_textureResources = createRandomizedTexture();
        setQuadResources(m_quadResources, *m_textureResources.textureSampler);

        m_meshNode.setAppearance(m_appearance);
        m_meshNode.setIndexCount(4);
        m_appearance.setDrawMode(ramses::EDrawMode_TriangleStrip);
        m_meshNode.setGeometryBinding(m_geometryBinding);

    }

    DynamicQuad_ClientResources::~DynamicQuad_ClientResources()
    {
        destroyQuadResources(m_quadResources);
        destroyTexture(m_textureResources);
    }

    void DynamicQuad_ClientResources::markSceneObjectsDestroyed()
    {
        m_textureResources.textureSampler = nullptr;
    }

    void DynamicQuad_ClientResources::createTextureDataConsumer(ramses::dataConsumerId_t textureSlotId)
    {
        m_scene.createTextureConsumer(*m_textureResources.textureSampler, textureSlotId);
        m_scene.flush();
    }

    void DynamicQuad_ClientResources::recreate()
    {
        const QuadResources oldQuad = m_quadResources;
        const TextureResources oldTexture = m_textureResources;
        m_quadResources = createRandomizedQuadResources();
        m_textureResources = createRandomizedTexture();
        setQuadResources(m_quadResources, *m_textureResources.textureSampler);

        destroyQuadResources(oldQuad);
        destroyTexture(oldTexture);
    }

    TextureResources DynamicQuad_ClientResources::createRandomizedTexture()
    {
        TextureResources resources;

        const uint32_t textureWidth = 64;
        const uint32_t textureHeight = 32;
        std::unique_ptr<uint8_t[]> rawData(new uint8_t[textureWidth * textureHeight * 3]);

        for (uint32_t x = 0; x < textureWidth; ++x)
        {
            for (uint32_t y = 0; y < textureHeight; ++y)
            {
                rawData[3 * (y * textureWidth + x) + 0] = static_cast<uint8_t>(TestRandom::Get(0, 255));
                rawData[3 * (y * textureWidth + x) + 1] = static_cast<uint8_t>(TestRandom::Get(0, 255));
                rawData[3 * (y * textureWidth + x) + 2] = static_cast<uint8_t>(TestRandom::Get(0, 255));
            }
        }

        ramses::MipLevelData textureData(textureWidth * textureHeight * 3, rawData.get());
        resources.texture = m_client.createTexture2D(textureWidth, textureHeight, ramses::ETextureFormat_RGB8, 1, &textureData, false, ramses::ResourceCacheFlag_DoNotCache);

        resources.textureSampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear_MipMapLinear,
            ramses::ETextureSamplingMethod_Linear,
            *resources.texture);

        return resources;
    }

    void DynamicQuad_ClientResources::destroyTexture(const TextureResources& textureResources)
    {
        m_client.destroy(*textureResources.texture);

        if (nullptr != textureResources.textureSampler)
        {
            m_scene.destroy(*textureResources.textureSampler);
        }
    }
}
