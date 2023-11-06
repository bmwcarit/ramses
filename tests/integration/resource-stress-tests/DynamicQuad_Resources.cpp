//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_Resources.h"

#include "ramses/client/Scene.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/TextureSampler.h"

#include "internal/PlatformAbstraction/PlatformMath.h"
#include "TestRandom.h"
#include <memory>

namespace ramses::internal
{
    DynamicQuad_Resources::DynamicQuad_Resources(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(scene, screenspaceQuad)
    {
        m_renderGroup.addMeshNode(m_meshNode);

        m_quadResources = createRandomizedQuadResources();
        m_textureResources = createRandomizedTexture();
        setQuadResources(m_quadResources, *m_textureResources.textureSampler);

        m_meshNode.setAppearance(m_appearance);
        m_meshNode.setIndexCount(4);
        m_appearance.setDrawMode(ramses::EDrawMode::TriangleStrip);
        m_meshNode.setGeometry(m_geometryBinding);

    }

    DynamicQuad_Resources::~DynamicQuad_Resources()
    {
        destroyQuadResources(m_quadResources);
        destroyTexture(m_textureResources);
    }

    void DynamicQuad_Resources::markSceneObjectsDestroyed()
    {
        m_textureResources.textureSampler = nullptr;
        m_textureResources.texture = nullptr;
        m_quadResources.indices = nullptr;
        m_quadResources.texCoords = nullptr;
        m_quadResources.vertexPos = nullptr;
    }

    void DynamicQuad_Resources::createTextureDataConsumer(ramses::dataConsumerId_t textureSlotId)
    {
        m_scene.createTextureConsumer(*m_textureResources.textureSampler, textureSlotId);
        m_scene.flush();
    }

    void DynamicQuad_Resources::recreate()
    {
        const QuadResources oldQuad = m_quadResources;
        const TextureResources oldTexture = m_textureResources;
        m_quadResources = createRandomizedQuadResources();
        m_textureResources = createRandomizedTexture();
        setQuadResources(m_quadResources, *m_textureResources.textureSampler);

        destroyQuadResources(oldQuad);
        destroyTexture(oldTexture);
    }

    TextureResources DynamicQuad_Resources::createRandomizedTexture()
    {
        TextureResources resources;

        const uint32_t textureWidth = 64;
        const uint32_t textureHeight = 32;
        MipLevelData rawData(textureWidth * textureHeight * 3);

        for (uint32_t x = 0; x < textureWidth; ++x)
        {
            for (uint32_t y = 0; y < textureHeight; ++y)
            {
                rawData[3 * (y * textureWidth + x) + 0] = static_cast<std::byte>(TestRandom::Get(0, 255));
                rawData[3 * (y * textureWidth + x) + 1] = static_cast<std::byte>(TestRandom::Get(0, 255));
                rawData[3 * (y * textureWidth + x) + 2] = static_cast<std::byte>(TestRandom::Get(0, 255));
            }
        }

        const std::vector<MipLevelData> textureData{ rawData };
        resources.texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, textureWidth, textureHeight, textureData, false, {});

        resources.textureSampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear_MipMapLinear,
            ramses::ETextureSamplingMethod::Linear,
            *resources.texture);

        return resources;
    }

    void DynamicQuad_Resources::destroyTexture(const TextureResources& textureResources)
    {
        if (textureResources.texture)
            m_scene.destroy(*textureResources.texture);

        if (textureResources.textureSampler)
            m_scene.destroy(*textureResources.textureSampler);
    }
}
