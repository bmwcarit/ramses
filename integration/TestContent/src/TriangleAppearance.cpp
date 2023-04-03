//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>

#include "PlatformAbstraction/PlatformTypes.h"

#include "TestScenes/TriangleAppearance.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/DataVector4f.h"

namespace ramses
{
    TriangleAppearance::TriangleAppearance(Scene& scene, const Effect& effect, enum TriangleAppearance::EColor color, float alpha)
        : m_appearance(createAppearance(effect, scene))
    {
        if (StatusOK == effect.findUniformInput("color", m_colorInput))
        {
            setColor(color, alpha);
        }
    }

    Appearance& TriangleAppearance::createAppearance(const Effect& effect, Scene& scene)
    {
        return *scene.createAppearance(effect, "appearance");
    }

    void TriangleAppearance::setColor(enum EColor color, float alpha)
    {
        status_t status = StatusOK;
        switch (color)
        {
        case EColor_Red:
            status = m_appearance.setInputValue(m_colorInput, vec4f{ 1.f, 0.f, 0.f, alpha });
            break;
        case EColor_Blue:
            status = m_appearance.setInputValue(m_colorInput, vec4f{ 0.f, 0.f, 1.f, alpha });
            break;
        case EColor_Green:
            status = m_appearance.setInputValue(m_colorInput, vec4f{ 0.f, 1.f, 0.f, alpha });
            break;
        case EColor_White:
            status = m_appearance.setInputValue(m_colorInput, vec4f{ 1.f, 1.f, 1.f, alpha });
            break;
        case EColor_Grey:
            status = m_appearance.setInputValue(m_colorInput, vec4f{ 0.5f, 0.5f, 0.5f, alpha });
            break;
        default:
            assert(false && "Chosen color for triangle is not available!");
            break;
        }

        assert(status == StatusOK);
        UNUSED(status);
    }

    void TriangleAppearance::bindColor(const DataVector4f& colorDataObject)
    {
        const status_t status = m_appearance.bindInput(m_colorInput, colorDataObject);
        assert(status == StatusOK);
        UNUSED(status);
    }

    void TriangleAppearance::unbindColor()
    {
        const status_t status = m_appearance.unbindInput(m_colorInput);
        assert(status == StatusOK);
        UNUSED(status);
    }
}
