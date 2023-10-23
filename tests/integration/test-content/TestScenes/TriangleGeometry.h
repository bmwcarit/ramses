//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/UniformInput.h"

namespace ramses
{
    class Scene;
    class Geometry;
    class ArrayResource;
    class ArrayResourceInput;
    class Effect;
    class UniformInput;
}

namespace ramses::internal
{
    class TriangleGeometry
    {
    public:
        enum EVerticesOrder
        {
            EVerticesOrder_CW = 0,
            EVerticesOrder_CCW
        };

        TriangleGeometry(ramses::Scene& scene, const Effect& effect, EVerticesOrder vertOrder = EVerticesOrder_CCW);

        Geometry& GetGeometry()
        {
            return m_geometry;
        }

    private:
        static Geometry& createGeometry(ramses::Scene& scene, const Effect& effect, const ArrayResource& indices);
        static const ArrayResource& createIndices(ramses::Scene& scene, EVerticesOrder vertOrder);

        const ArrayResource& m_indices;
        Geometry&   m_geometry;
    };
}
