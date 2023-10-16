//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TriangleGeometry.h"

#include "ramses/client/Scene.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/framework/DataTypes.h"

#include <cstdint>
#include <cassert>

namespace ramses::internal
{
    TriangleGeometry::TriangleGeometry(ramses::Scene& scene, const Effect& effect, EVerticesOrder vertOrder)
        : m_indices(createIndices(scene, vertOrder))
        , m_geometry(createGeometry(scene, effect, m_indices))
    {
    }

    Geometry& TriangleGeometry::createGeometry(ramses::Scene& scene, const Effect& effect, const ArrayResource& indices)
    {
        Geometry* geometry = scene.createGeometry(effect, "triangle geometry");

        geometry->setIndices(indices);
        const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
        const ArrayResource* vertexPositions = scene.createArrayResource(3u, vertexPositionsData.data());
        geometry->setInputBuffer(*effect.findAttributeInput("a_position"), *vertexPositions);

        return *geometry;
    }

    const ArrayResource& TriangleGeometry::createIndices(ramses::Scene& scene, EVerticesOrder vertOrder)
    {
        const std::array<uint16_t, 3> indiceData_ccw = { 0, 1, 2 };
        const std::array<uint16_t, 3> indiceData_cw = { 0, 2, 1 };
        const uint16_t* indiceData = (vertOrder == EVerticesOrder_CCW ? indiceData_ccw.data() : indiceData_cw.data());
        return *scene.createArrayResource(3u, indiceData);
    }
}
