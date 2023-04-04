//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>

#include "PlatformAbstraction/PlatformTypes.h"

#include "TestScenes/MultiTriangleGeometry.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"

namespace ramses
{
    MultiTriangleGeometry::MultiTriangleGeometry(Scene& scene, Effect& effect, enum MultiTriangleGeometry::EColor color, float alpha, EGeometryType geometryType, EVerticesOrder vertOrder)
        : m_appearance(createAppearance(effect, scene))
        , m_indices(createIndices(scene, vertOrder))
        , m_geometry(createGeometry(scene, effect, m_indices, geometryType))
    {
        if (geometryType == EGeometryType_TriangleStripQuad)
        {
            m_appearance.setDrawMode(ramses::EDrawMode_TriangleStrip);
        }
        else
        {
            m_appearance.setDrawMode(ramses::EDrawMode_TriangleFan);
        }
        UniformInput colorInput;
        if (StatusOK == effect.findUniformInput("color", colorInput))
        {
            setColor(colorInput, color, alpha);
        }
    }

    Appearance& MultiTriangleGeometry::createAppearance(Effect& effect, Scene& scene)
    {
        return *scene.createAppearance(effect, "appearance");
    }

    GeometryBinding& MultiTriangleGeometry::createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices, EGeometryType geometryType)
    {
        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        GeometryBinding* geometry = scene.createGeometryBinding(effect, "triangle geometry");

        geometry->setIndices(indices);
        const ramses::vec3f* vertexPositionsData = nullptr;
        const std::array<ramses::vec3f, 4u> vertexPositionsDataTriangleStrip{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{-1.f, -1.f, -1.f}, ramses::vec3f{0.f, 0.f, -1.f}, ramses::vec3f{0.f, -1.f, -1.f} };
        const std::array<ramses::vec3f, 4u> vertexPositionsDataTriangleFan{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{-1.f, -1.f, -1.f}, ramses::vec3f{-0.2f, -0.7f, -1.f}, ramses::vec3f{0.f, 0.f, -1.f} };
        if (geometryType == EGeometryType_TriangleStripQuad)
            vertexPositionsData = vertexPositionsDataTriangleStrip.data();
        else
            vertexPositionsData = vertexPositionsDataTriangleFan.data();
        const ArrayResource* vertexPositions = scene.createArrayResource(4u, vertexPositionsData);
        geometry->setInputBuffer(positionsInput, *vertexPositions);

        return *geometry;
    }

    const ArrayResource& MultiTriangleGeometry::createIndices(Scene& scene, EVerticesOrder vertOrder)
    {
        static const uint16_t indiceData_ccw[] = {0, 1, 2, 3};
        static const uint16_t indiceData_cw[]  = {0, 2, 1, 3};
        const uint16_t* indiceData = (vertOrder == EVerticesOrder_CCW ? indiceData_ccw : indiceData_cw);
        return *scene.createArrayResource(4, indiceData);
    }

    void MultiTriangleGeometry::setColor(const UniformInput& colorInput, enum EColor color, float alpha)
    {
        status_t status = StatusOK;
        switch (color)
        {
        case ramses::MultiTriangleGeometry::EColor_Red:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 0.f, 0.f, alpha });
            break;
        case ramses::MultiTriangleGeometry::EColor_Blue:
            status = m_appearance.setInputValue(colorInput, vec4f{ 0.f, 0.f, 1.f, alpha });
            break;
        case ramses::MultiTriangleGeometry::EColor_Green:
            status = m_appearance.setInputValue(colorInput, vec4f{ 0.f, 1.f, 0.f, alpha });
            break;
        case ramses::MultiTriangleGeometry::EColor_White:
            status = m_appearance.setInputValue(colorInput, vec4f{ 1.f, 1.f, 1.f, alpha });
            break;
        default:
            assert(false && "Chosen color for triangle is not available!");
            break;
        }

        assert(status == StatusOK);
        UNUSED(status);
    }
}
