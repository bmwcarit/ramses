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
    class RamsesClient;
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
    class TriangleAppearance
    {
    public:
        enum class EColor
        {
            Red,
            Blue,
            Green,
            White,
            Grey,
            None // won't set any color
        };

        TriangleAppearance(ramses::Scene& scene, const Effect& effect, enum EColor color, float alpha = 1.f);

        Appearance& GetAppearance()
        {
            return m_appearance;
        }

        void setColor(enum EColor color, float alpha);
        void bindColor(const DataObject& colorDataObject);
        void unbindColor();

    private:
        static Appearance& createAppearance(const Effect& effect, ramses::Scene& scene);

        std::optional<UniformInput> m_colorInput;
        Appearance&                 m_appearance;
    };
}
