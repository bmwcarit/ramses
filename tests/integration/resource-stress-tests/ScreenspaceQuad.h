//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Scene.h"
#include "impl/DataTypesImpl.h"
#include "TestRandom.h"

namespace ramses::internal
{
    enum class EScreenspaceQuadVertex
    {
        BottomLeft = 0,
        BottomRight,
        TopRight,
        TopLeft
    };

    struct ScreenspaceQuad
    {
        ScreenspaceQuad(uint32_t screenWidth, uint32_t screenHeight, const glm::vec4& quadRegionAsFloats = {0.0f, 0.0f, 1.0f, 1.0f})
            : m_widthOfScreen(screenWidth)
            , m_heightOfScreen(screenHeight)
            , m_quadX       (static_cast<uint32_t>(quadRegionAsFloats.x * m_widthOfScreen))
            , m_quadY       (static_cast<uint32_t>(quadRegionAsFloats.y * m_heightOfScreen))
            , m_quadWidth   (static_cast<uint32_t>(quadRegionAsFloats.z * m_widthOfScreen) - m_quadX)
            , m_quadHeight  (static_cast<uint32_t>(quadRegionAsFloats.w * m_heightOfScreen) - m_quadY)
        {
        }

        [[nodiscard]] ScreenspaceQuad createSubQuad(const glm::vec4& quadSubregionAsFloats) const
        {
            glm::vec4 subQuadCoords(
                (m_quadX + quadSubregionAsFloats.x * m_quadWidth)   / m_widthOfScreen,
                (m_quadY + quadSubregionAsFloats.y * m_quadHeight)  / m_heightOfScreen,
                (m_quadX + quadSubregionAsFloats.z * m_quadWidth)   / m_widthOfScreen,
                (m_quadY + quadSubregionAsFloats.w * m_quadHeight)  / m_heightOfScreen
            );

            return ScreenspaceQuad(m_widthOfScreen, m_heightOfScreen, subQuadCoords);
        }

        [[nodiscard]] glm::vec3 getVertex(EScreenspaceQuadVertex vertex, size_t jitterRange) const
        {
            glm::vec3 screenspaceVertexWithNoise(0.f);

            switch (vertex)
            {
            case EScreenspaceQuadVertex::BottomLeft:
                screenspaceVertexWithNoise = glm::vec3(static_cast<float>(m_quadX), static_cast<float>(m_quadY), -0.5f);
                break;
            case EScreenspaceQuadVertex::BottomRight:
                screenspaceVertexWithNoise = glm::vec3(static_cast<float>(m_quadX + m_quadWidth), static_cast<float>(m_quadY), -0.5f);
                break;
            case EScreenspaceQuadVertex::TopRight:
                screenspaceVertexWithNoise = glm::vec3(static_cast<float>(m_quadX + m_quadWidth), static_cast<float>(m_quadY + m_quadHeight), -0.5f);
                break;
            case EScreenspaceQuadVertex::TopLeft:
                screenspaceVertexWithNoise = glm::vec3(static_cast<float>(m_quadX), static_cast<float>(m_quadY + m_quadHeight), -0.5f);
                break;
            }

            screenspaceVertexWithNoise.x += TestRandom::Get(0, jitterRange);
            screenspaceVertexWithNoise.y += TestRandom::Get(0, jitterRange);

            return screenspaceVertexWithNoise;
        }

        [[nodiscard]] uint32_t getWidthOfScreen() const
        {
            return m_widthOfScreen;
        }

        [[nodiscard]] uint32_t getHeightOfScreen() const
        {
            return m_heightOfScreen;
        }

        [[nodiscard]] ramses::OrthographicCamera& createOrthoCamera(ramses::Scene& scene) const
        {
            ramses::Node* cameraTranslation = scene.createNode();
            cameraTranslation->setTranslation({0.0f, 0.0f, 0.5f});

            ramses::OrthographicCamera& camera = *scene.createOrthographicCamera();
            camera.setFrustum(0.0f, static_cast<float>(m_widthOfScreen), 0.0f, static_cast<float>(m_heightOfScreen), 0.1f, 100.f);
            camera.setViewport(0, 0, m_widthOfScreen, m_heightOfScreen);
            camera.setParent(*cameraTranslation);

            return camera;
        }

    private:
        // Screen size in pixels (==Viewport)
        uint32_t m_widthOfScreen;
        uint32_t m_heightOfScreen;
        // Quad position and size in pixels (within above screenspace)
        uint32_t m_quadX;
        uint32_t m_quadY;
        uint32_t m_quadWidth;
        uint32_t m_quadHeight;
    };
}
