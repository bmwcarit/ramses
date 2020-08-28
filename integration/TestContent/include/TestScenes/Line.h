//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINE_H
#define RAMSES_LINE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/AppearanceEnums.h"

namespace ramses
{
    class Scene;
    class Appearance;
    class GeometryBinding;
    class ArrayResource;
    class ArrayResourceInput;
    class Effect;
    class UniformInput;

    class Line
    {
    public:
        enum EColor
        {
            EColor_Red = 0,
            EColor_Blue,
            EColor_Green,
            EColor_White,
            EColor_Yellow
        };

        Line(Scene& scene, Effect& effect, enum EColor color, EDrawMode desiredDrawMode, float alpha = 1.f);

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
        static GeometryBinding& createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices);
        static const ArrayResource& createIndices(Scene& scene, EDrawMode desiredDrawMode);

        void setColor(const UniformInput& colorInput, enum EColor color, float alpha);

        Appearance&        m_appearance;
        const ArrayResource& m_indices;
        GeometryBinding&   m_geometry;
    };
}

#endif
