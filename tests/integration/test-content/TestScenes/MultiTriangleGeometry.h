//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"

namespace ramses
{
    class Scene;
    class Appearance;
    class Geometry;
    class ArrayResource;
    class ArrayResourceInput;
    class Effect;
    class UniformInput;
}

namespace ramses::internal
{
    class MultiTriangleGeometry
    {
    public:
        enum EColor
        {
            EColor_Red = 0,
            EColor_Blue,
            EColor_Green,
            EColor_White
        };

        enum EGeometryType
        {
            EGeometryType_TriangleStripQuad = 0,
            EGeometryType_TriangleFan
        };

        enum EVerticesOrder
        {
            EVerticesOrder_CW = 0,
            EVerticesOrder_CCW
        };

        MultiTriangleGeometry(Scene& scene, Effect& effect, enum EColor color, float alpha = 1.f, EGeometryType geometryType = EGeometryType_TriangleStripQuad, EVerticesOrder vertOrder = EVerticesOrder_CCW);

        Appearance& GetAppearance()
        {
            return m_appearance;
        }
        Geometry& GetGeometry()
        {
            return m_geometry;
        }

    protected:
        static Appearance&        createAppearance(Effect& effect, Scene& scene);
        static Geometry&   createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices, EGeometryType geometryType);
        static const ArrayResource& createIndices(Scene& scene, EVerticesOrder vertOrder);

        void setColor(const UniformInput& colorInput, enum EColor color, float alpha);

        Appearance&        m_appearance;
        const ArrayResource& m_indices;
        Geometry&   m_geometry;
    };
}
