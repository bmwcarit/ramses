//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRIANGLEGEOMETRY_H
#define RAMSES_TRIANGLEGEOMETRY_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/UniformInput.h"

namespace ramses
{
    class Scene;
    class GeometryBinding;
    class ArrayResource;
    class ArrayResourceInput;
    class Effect;
    class UniformInput;

    class TriangleGeometry
    {
    public:
        enum EVerticesOrder
        {
            EVerticesOrder_CW = 0,
            EVerticesOrder_CCW
        };

        TriangleGeometry(Scene& scene, const Effect& effect, EVerticesOrder vertOrder = EVerticesOrder_CCW);

        GeometryBinding& GetGeometry()
        {
            return m_geometry;
        }

    private:
        static GeometryBinding&   createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices);
        static const ArrayResource& createIndices(Scene& scene, EVerticesOrder vertOrder);

        const ArrayResource& m_indices;
        GeometryBinding&   m_geometry;
    };
}

#endif
