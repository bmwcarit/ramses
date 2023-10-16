//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultiTriangleGeometry.h"

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
    MultiTriangleGeometry::MultiTriangleGeometry(Scene& scene, Effect& effect, enum MultiTriangleGeometry::EColor color, float alpha, EGeometryType geometryType, EVerticesOrder vertOrder)
        : m_appearance(createAppearance(effect, scene))
        , m_indices(createIndices(scene, vertOrder))
        , m_geometry(createGeometry(scene, effect, m_indices, geometryType))
    {
        if (geometryType == EGeometryType_TriangleStripQuad)
        {
            m_appearance.setDrawMode(ramses::EDrawMode::TriangleStrip);
        }
        else
        {
            m_appearance.setDrawMode(ramses::EDrawMode::TriangleFan);
        }
        std::optional<UniformInput> colorInput = effect.findUniformInput("color");
        assert(colorInput.has_value());
        setColor(*colorInput, color, alpha);
    }

    Appearance& MultiTriangleGeometry::createAppearance(Effect& effect, Scene& scene)
    {
        return *scene.createAppearance(effect, "appearance");
    }

    Geometry& MultiTriangleGeometry::createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices, EGeometryType geometryType)
    {
        Geometry* geometry = scene.createGeometry(effect, "triangle geometry");

        geometry->setIndices(indices);
        const ramses::vec3f* vertexPositionsData = nullptr;
        const std::array<ramses::vec3f, 4u> vertexPositionsDataTriangleStrip{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{-1.f, -1.f, -1.f}, ramses::vec3f{0.f, 0.f, -1.f}, ramses::vec3f{0.f, -1.f, -1.f} };
        const std::array<ramses::vec3f, 4u> vertexPositionsDataTriangleFan{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{-1.f, -1.f, -1.f}, ramses::vec3f{-0.2f, -0.7f, -1.f}, ramses::vec3f{0.f, 0.f, -1.f} };
        if (geometryType == EGeometryType_TriangleStripQuad)
        {
            vertexPositionsData = vertexPositionsDataTriangleStrip.data();
        }
        else
        {
            vertexPositionsData = vertexPositionsDataTriangleFan.data();
        }
        const ArrayResource* vertexPositions = scene.createArrayResource(4u, vertexPositionsData);
        geometry->setInputBuffer(*effect.findAttributeInput("a_position"), *vertexPositions);

        return *geometry;
    }

    const ArrayResource& MultiTriangleGeometry::createIndices(Scene& scene, EVerticesOrder vertOrder)
    {
        const std::array<uint16_t, 4> indiceData_ccw = {0, 1, 2, 3};
        const std::array<uint16_t, 4> indiceData_cw  = {0, 2, 1, 3};
        const uint16_t* indiceData = (vertOrder == EVerticesOrder_CCW ? indiceData_ccw.data() : indiceData_cw.data());
        return *scene.createArrayResource(4, indiceData);
    }

    void MultiTriangleGeometry::setColor(const UniformInput& colorInput, enum EColor color, float alpha)
    {
        [[maybe_unused]] bool status = false;
        switch (color)
        {
        case MultiTriangleGeometry::EColor_Red:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 0.f, 0.f, alpha });
            break;
        case MultiTriangleGeometry::EColor_Blue:
            status = m_appearance.setInputValue(colorInput, vec4f{ 0.f, 0.f, 1.f, alpha });
            break;
        case MultiTriangleGeometry::EColor_Green:
            status = m_appearance.setInputValue(colorInput, vec4f{ 0.f, 1.f, 0.f, alpha });
            break;
        case MultiTriangleGeometry::EColor_White:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 1.f, 1.f, alpha });
            break;
        default:
            assert(false && "Chosen color for triangle is not available!");
            break;
        }

        assert(status);
    }
}
