//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRIANGLESTRIPQUAD_H
#define RAMSES_TRIANGLESTRIPQUAD_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class RamsesClient;
    class Scene;
    class Appearance;
    class GeometryBinding;
    class UInt16Array;
    class Vector3fArrayInput;
    class Effect;
    class UniformInput;

    class TriangleStripQuad
    {
    public:
        enum EColor
        {
            EColor_Red = 0,
            EColor_Blue,
            EColor_Green,
            EColor_White
        };

        enum EVerticesOrder
        {
            EVerticesOrder_CW = 0,
            EVerticesOrder_CCW
        };

        TriangleStripQuad(RamsesClient& client, Scene& scene, Effect& effect, enum EColor color, float alpha = 1.f, EVerticesOrder vertOrder = EVerticesOrder_CCW);

        Appearance& GetAppearance()
        {
            return m_appearance;
        }
        GeometryBinding& GetGeometry()
        {
            return m_geometry;
        }

    protected:
        static Appearance&        createAppearance(Effect& effect, Scene& scene);
        static GeometryBinding&   createGeometry(RamsesClient& client, Scene& scene, const Effect& effect, const UInt16Array& indices);
        static const UInt16Array& createIndices(RamsesClient& client, EVerticesOrder vertOrder);

        void setColor(const UniformInput& colorInput, enum EColor color, float alpha);

        Appearance&        m_appearance;
        const UInt16Array& m_indices;
        GeometryBinding&   m_geometry;
    };
}

#endif
