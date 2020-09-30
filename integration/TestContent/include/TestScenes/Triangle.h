//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRIANGLE_H
#define RAMSES_TRIANGLE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/UniformInput.h"

#include "TestScenes/TriangleAppearance.h"
#include "TestScenes/TriangleGeometry.h"

namespace ramses
{
    class Scene;
    class Appearance;
    class GeometryBinding;
    class ArrayResource;
    class ArrayResourceInput;
    class Effect;
    class UniformInput;
    class DataVector4f;

    class Triangle
    {
    public:
        Triangle(Scene& scene, const Effect& effect, enum TriangleAppearance::EColor color, float alpha = 1.f, TriangleGeometry::EVerticesOrder vertOrder = TriangleGeometry::EVerticesOrder_CCW);

        Appearance& GetAppearance()
        {
            return m_appearance.GetAppearance();
        }
        GeometryBinding& GetGeometry()
        {
            return m_geometry.GetGeometry();
        }

        void bindColor(const DataVector4f& colorDataObject);
        void unbindColor();

    private:
        TriangleAppearance m_appearance;
        TriangleGeometry   m_geometry;
    };
}

#endif
