//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <assert.h>

#include "PlatformAbstraction/PlatformTypes.h"

#include "TestScenes/TriangleGeometry.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/DataVector4f.h"

namespace ramses
{
    TriangleGeometry::TriangleGeometry(RamsesClient& client, Scene& scene, const Effect& effect, EVerticesOrder vertOrder)
        : m_indices(createIndices(client, vertOrder))
        , m_geometry(createGeometry(client, scene, effect, m_indices))
    {
    }

    GeometryBinding& TriangleGeometry::createGeometry(RamsesClient& client, Scene& scene, const Effect& effect, const UInt16Array& indices)
    {
        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        GeometryBinding* geometry = scene.createGeometryBinding(effect, "triangle geometry");

        geometry->setIndices(indices);
        const float vertexPositionsData[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, -1.f };
        const Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsData);
        geometry->setInputBuffer(positionsInput, *vertexPositions);

        return *geometry;
    }

    const UInt16Array& TriangleGeometry::createIndices(RamsesClient& client, EVerticesOrder vertOrder)
    {
        const uint16_t indiceData_ccw[] = { 0, 1, 2 };
        const uint16_t indiceData_cw[] = { 0, 2, 1 };
        const uint16_t* indiceData = (vertOrder == EVerticesOrder_CCW ? indiceData_ccw : indiceData_cw);
        return *client.createConstUInt16Array(3, indiceData);
    }
}
