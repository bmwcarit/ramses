//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/AppearanceEnums.h"

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
        Geometry& GetGeometry()
        {
            return m_geometry;
        }

    protected:
        static Appearance&        createAppearance(Effect& effect, Scene& scene);
        static Geometry& createGeometry(Scene& scene, const Effect& effect, const ArrayResource& indices);
        static const ArrayResource& createIndices(Scene& scene, EDrawMode desiredDrawMode);

        void setColor(const UniformInput& colorInput, enum EColor color, float alpha);

        Appearance&        m_appearance;
        const ArrayResource& m_indices;
        Geometry&   m_geometry;
    };
}
