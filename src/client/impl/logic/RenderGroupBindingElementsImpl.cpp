//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/RenderGroupBindingElementsImpl.h"
#include "internal/Core/Utils/LogMacros.h"
#include "ramses/client/SceneObject.h"
#include "impl/SceneObjectImpl.h"
#include <cassert>

namespace ramses::internal
{
    bool RenderGroupBindingElementsImpl::addElement(const ramses::SceneObject& ramsesObject, std::string_view elementName)
    {
        assert(ramsesObject.isOfType(ramses::ERamsesObjectType::MeshNode) || ramsesObject.isOfType(ramses::ERamsesObjectType::RenderGroup));

        if (!m_elements.empty() && !m_elements.front().second->impl().isFromTheSameSceneAs(ramsesObject.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RenderGroupBindingElements: Failed to add element, element is from different Scene than those already added.");
            return false;
        }

        std::string name{ elementName.empty() ? ramsesObject.getName() : elementName };
        if (name.empty())
        {
            LOG_ERROR(CONTEXT_CLIENT, "RenderGroupBindingElements: Failed to add element, object has no name and provided element name is empty.");
            return false;
        }

        const auto it = std::find_if(m_elements.cbegin(), m_elements.cend(), [&ramsesObject](const auto& e) { return e.second == &ramsesObject; });
        if (it != m_elements.cend())
        {
            LOG_ERROR(CONTEXT_CLIENT, "RenderGroupBindingElements: Failed to add element '{}', it is already contained under name '{}'.", name, it->first);
            return false;
        }

        m_elements.push_back({ std::move(name), &ramsesObject });

        return true;
    }

    const RenderGroupBindingElementsImpl::Elements& RenderGroupBindingElementsImpl::getElements() const
    {
        return m_elements;
    }
}
