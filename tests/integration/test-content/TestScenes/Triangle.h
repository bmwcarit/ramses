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

#include "TestScenes/TriangleAppearance.h"
#include "TestScenes/TriangleGeometry.h"

namespace ramses
{
    class Scene;
    class Appearance;
    class Geometry;
    class ArrayResource;
    class ArrayResourceInput;
    class Effect;
    class UniformInput;
    class DataObject;
}

namespace ramses::internal
{
    class Triangle
    {
    public:
        Triangle(ramses::Scene& scene, const Effect& effect, enum TriangleAppearance::EColor color, float alpha = 1.f, TriangleGeometry::EVerticesOrder vertOrder = TriangleGeometry::EVerticesOrder_CCW);

        Appearance& GetAppearance()
        {
            return m_appearance.GetAppearance();
        }
        Geometry& GetGeometry()
        {
            return m_geometry.GetGeometry();
        }

        void bindColor(const DataObject& colorDataObject);
        void unbindColor();

    private:
        TriangleAppearance m_appearance;
        TriangleGeometry   m_geometry;
    };
}
