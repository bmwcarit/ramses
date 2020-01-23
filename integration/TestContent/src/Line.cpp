//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <assert.h>

#include "PlatformAbstraction/PlatformTypes.h"

#include "TestScenes/Line.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"

namespace ramses
{
    Line::Line(RamsesClient& client, Scene& scene, Effect& effect, enum Line::EColor color, EDrawMode desiredDrawMode, float alpha)
        : m_appearance(createAppearance(effect, scene))
        , m_indices(createIndices(client, desiredDrawMode))
        , m_geometry(createGeometry(client, scene, effect, m_indices))
    {
        m_appearance.setDrawMode(desiredDrawMode);
        UniformInput colorInput;
        effect.findUniformInput("color", colorInput);
        setColor(colorInput, color, alpha);
    }

    Appearance& Line::createAppearance(Effect& effect, Scene& scene)
    {
        return *scene.createAppearance(effect, "appearance");
    }

    GeometryBinding& Line::createGeometry(RamsesClient& client, Scene& scene, const Effect& effect, const UInt16Array& indices)
    {
        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        GeometryBinding* geometry = scene.createGeometryBinding(effect, "triangle geometry");

        geometry->setIndices(indices);
        static const float vertexPositionsData[] =
        {
            -1.f, 0.f, -1.f,
            0.f, 1.f, -1.f,
            -0.75f, -0.25f, -1.f,
            0.25f, 0.75f, -1.f,
            -0.5f, -0.5f, -1.f,
            0.5f, 0.5f, -1.f,
            -0.25f, -0.75f, -1.f,
            0.75f, 0.25f, -1.f,
            0.f, -1.f, -1.f,
            1.f, 0.f, -1.f
        };
        const Vector3fArray* vertexPositions = client.createConstVector3fArray(10, vertexPositionsData);
        geometry->setInputBuffer(positionsInput, *vertexPositions);

        return *geometry;
    }

    const UInt16Array& Line::createIndices(RamsesClient& client, EDrawMode desiredDrawMode)
    {
        static const uint16_t indiceData_line[] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9};
        static const uint16_t indiceData_lineStrip[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        switch (desiredDrawMode)
        {
        case ramses::EDrawMode_Lines:
            return *client.createConstUInt16Array(18, indiceData_line);
            break;
        case ramses::EDrawMode_Points:
        case ramses::EDrawMode_LineStrip:
            return *client.createConstUInt16Array(10, indiceData_lineStrip);
            break;
        default:
            assert(false && "not supported");
            return *client.createConstUInt16Array(10, indiceData_lineStrip);
        }
    }

    void Line::setColor(const UniformInput& colorInput, enum EColor color, float alpha)
    {
        status_t status = StatusOK;
        switch (color)
        {
        case ramses::Line::EColor_Red:
            status = m_appearance.setInputValueVector4f(colorInput, 1.f, 0.f, 0.f, alpha);
            break;
        case ramses::Line::EColor_Blue:
            status = m_appearance.setInputValueVector4f(colorInput, 0.f, 0.f, 1.f, alpha);
            break;
        case ramses::Line::EColor_Green:
            status = m_appearance.setInputValueVector4f(colorInput, 0.f, 1.0, 0.f, alpha);
            break;
        case ramses::Line::EColor_White:
            status = m_appearance.setInputValueVector4f(colorInput, 1.f, 1.0, 1.f, alpha);
            break;
        case ramses::Line::EColor_Yellow:
            status = m_appearance.setInputValueVector4f(colorInput, 1.f, 1.f, 0.f, alpha);
            break;
        default:
            assert(false && "Chosen color for triangle is not available!");
            break;
        }

        assert(status == StatusOK);
        UNUSED(status);
    }
}
