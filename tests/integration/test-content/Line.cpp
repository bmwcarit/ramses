//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Line.h"

#include "ramses/client/Scene.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"

#include <cstdint>
#include <cassert>

namespace ramses::internal
{
    Line::Line(Scene& scene, Effect& effect, enum Line::EColor color, EDrawMode desiredDrawMode, float alpha)
        : m_appearance(createAppearance(effect, scene))
        , m_indices(createIndices(scene, desiredDrawMode))
        , m_geometry(createGeometry(scene, effect, m_indices))
    {
        m_appearance.setDrawMode(desiredDrawMode);
        setColor(*effect.findUniformInput("color"), color, alpha);
    }

    Appearance& Line::createAppearance(Effect& effect, Scene& scene)
    {
        return *scene.createAppearance(effect, "appearance");
    }

    ramses::Geometry& Line::createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices)
    {
        Geometry* geometry = scene.createGeometry(effect, "triangle geometry");

        geometry->setIndices(indices);
        const std::array<ramses::vec3f, 10u> vertexPositionsData
        {
            ramses::vec3f{ -1.f, 0.f, -1.f },
            ramses::vec3f{ 0.f, 1.f, -1.f },
            ramses::vec3f{ -0.75f, -0.25f, -1.f },
            ramses::vec3f{ 0.25f, 0.75f, -1.f },
            ramses::vec3f{ -0.5f, -0.5f, -1.f },
            ramses::vec3f{ 0.5f, 0.5f, -1.f },
            ramses::vec3f{ -0.25f, -0.75f, -1.f },
            ramses::vec3f{ 0.75f, 0.25f, -1.f },
            ramses::vec3f{ 0.f, -1.f, -1.f },
            ramses::vec3f{ 1.f, 0.f, -1.f }
        };
        const ArrayResource* vertexPositions = scene.createArrayResource(10u, vertexPositionsData.data());
        geometry->setInputBuffer(*effect.findAttributeInput("a_position"), *vertexPositions);

        return *geometry;
    }

    const ArrayResource& Line::createIndices(Scene& scene, EDrawMode desiredDrawMode)
    {
        const std::array<uint16_t, 18> indiceData_line = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9};
        const std::array<uint16_t, 10> indiceData_lineStrip = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        switch (desiredDrawMode)
        {
        case ramses::EDrawMode::Lines:
            return *scene.createArrayResource(18u, indiceData_line.data());
            break;
        case ramses::EDrawMode::Points:
        case ramses::EDrawMode::LineStrip:
            return *scene.createArrayResource(10u, indiceData_lineStrip.data());
            break;
        default:
            assert(false && "not supported");
            return *scene.createArrayResource(10u, indiceData_lineStrip.data());
        }
    }

    void Line::setColor(const UniformInput& colorInput, enum EColor color, float alpha)
    {
        [[maybe_unused]] bool status = false;
        switch (color)
        {
        case Line::EColor_Red:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 0.f, 0.f, alpha });
            break;
        case Line::EColor_Blue:
            status = m_appearance.setInputValue(colorInput, vec4f{ 0.f, 0.f, 1.f, alpha });
            break;
        case Line::EColor_Green:
            status = m_appearance.setInputValue(colorInput, vec4f{ 0.f, 1.f, 0.f, alpha });
            break;
        case Line::EColor_White:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 1.f, 1.f, alpha });
            break;
        case Line::EColor_Yellow:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 1.f, 0.f, alpha });
            break;
        default:
            assert(false && "Chosen color for triangle is not available!");
            break;
        }

        assert(status);
    }
}
