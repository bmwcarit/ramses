//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>

#include "PlatformAbstraction/PlatformTypes.h"

#include "TestScenes/TriangleGeometry.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    TriangleGeometry::TriangleGeometry(Scene& scene, const Effect& effect, EVerticesOrder vertOrder)
        : m_indices(createIndices(scene, vertOrder))
        , m_geometry(createGeometry(scene, effect, m_indices))
    {
    }

    GeometryBinding& TriangleGeometry::createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices)
    {
        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        GeometryBinding* geometry = scene.createGeometryBinding(effect, "triangle geometry");

        geometry->setIndices(indices);
        const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
        const ArrayResource* vertexPositions = scene.createArrayResource(3u, vertexPositionsData.data());
        geometry->setInputBuffer(positionsInput, *vertexPositions);

        return *geometry;
    }

    const ArrayResource& TriangleGeometry::createIndices(Scene& scene, EVerticesOrder vertOrder)
    {
        const uint16_t indiceData_ccw[] = { 0, 1, 2 };
        const uint16_t indiceData_cw[] = { 0, 2, 1 };
        const uint16_t* indiceData = (vertOrder == EVerticesOrder_CCW ? indiceData_ccw : indiceData_cw);
        return *scene.createArrayResource(3u, indiceData);
    }
}
