//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRIANGLEAPPEARANCE_H
#define RAMSES_TRIANGLEAPPEARANCE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/UniformInput.h"

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
    class DataVector4f;

    class TriangleAppearance
    {
    public:
        enum EColor
        {
            EColor_Red = 0,
            EColor_Blue,
            EColor_Green,
            EColor_White,
            EColor_Grey
        };

        TriangleAppearance(Scene& scene, const Effect& effect, enum EColor color, float alpha = 1.f);

        Appearance& GetAppearance()
        {
            return m_appearance;
        }

        void setColor(enum EColor color, float alpha);
        void bindColor(const DataVector4f& colorDataObject);
        void unbindColor();

    private:
        static Appearance& createAppearance(const Effect& effect, Scene& scene);

        UniformInput       m_colorInput;
        Appearance&        m_appearance;
    };
}

#endif
