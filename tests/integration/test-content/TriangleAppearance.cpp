//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TriangleAppearance.h"

#include "ramses/client/Scene.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/DataObject.h"

#include <cstdint>
#include <cassert>

namespace ramses::internal
{
    TriangleAppearance::TriangleAppearance(ramses::Scene& scene, const Effect& effect, enum TriangleAppearance::EColor color, float alpha)
        : m_appearance(createAppearance(effect, scene))
    {
        if (color != EColor::None)
        {
            m_colorInput = effect.findUniformInput("color");
            setColor(color, alpha);
        }
    }

    Appearance& TriangleAppearance::createAppearance(const Effect& effect, ramses::Scene& scene)
    {
        return *scene.createAppearance(effect, "appearance");
    }

    void TriangleAppearance::setColor(enum EColor color, float alpha)
    {
        if (!m_colorInput)
            m_colorInput = m_appearance.getEffect().findUniformInput("color");
        assert(m_colorInput.has_value());

        [[maybe_unused]] bool status = false;
        switch (color)
        {
        case EColor::Red:
            status = m_appearance.setInputValue(*m_colorInput, vec4f{ 1.f, 0.f, 0.f, alpha });
            break;
        case EColor::Blue:
            status = m_appearance.setInputValue(*m_colorInput, vec4f{ 0.f, 0.f, 1.f, alpha });
            break;
        case EColor::Green:
            status = m_appearance.setInputValue(*m_colorInput, vec4f{ 0.f, 1.f, 0.f, alpha });
            break;
        case EColor::White:
            status = m_appearance.setInputValue(*m_colorInput, vec4f{ 1.f, 1.f, 1.f, alpha });
            break;
        case EColor::Grey:
            status = m_appearance.setInputValue(*m_colorInput, vec4f{ 0.5f, 0.5f, 0.5f, alpha });
            break;
        case EColor::None:
            break;
        }

        assert(status);
    }

    void TriangleAppearance::bindColor(const DataObject& colorDataObject)
    {
        assert(m_colorInput.has_value());
        [[maybe_unused]] const bool status = m_appearance.bindInput(*m_colorInput, colorDataObject);
        assert(status);
    }

    void TriangleAppearance::unbindColor()
    {
        assert(m_colorInput.has_value());
        [[maybe_unused]] const bool status = m_appearance.unbindInput(*m_colorInput);
        assert(status);
    }
}
