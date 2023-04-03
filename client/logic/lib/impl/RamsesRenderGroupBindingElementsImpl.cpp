//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesRenderGroupBindingElementsImpl.h"
#include "impl/LoggerImpl.h"
#include "ramses-client-api/SceneObject.h"

namespace rlogic::internal
{
    bool RamsesRenderGroupBindingElementsImpl::addElement(const ramses::SceneObject& ramsesObject, std::string_view elementName)
    {
        assert(ramsesObject.isOfType(ramses::ERamsesObjectType_MeshNode) || ramsesObject.isOfType(ramses::ERamsesObjectType_RenderGroup));

        const std::string name = (elementName.empty() ? ramsesObject.getName() : std::string{ elementName });
        if (name.empty())
        {
            LOG_ERROR("RamsesRenderGroupBindingElements: Failed to add element, object has no name and provided element name is empty.");
            return false;
        }

        const auto it = std::find_if(m_elements.cbegin(), m_elements.cend(), [&ramsesObject](const auto& e) { return e.second == &ramsesObject; });
        if (it != m_elements.cend())
        {
            LOG_ERROR("RamsesRenderGroupBindingElements: Failed to add element '{}', it is already contained under name '{}'.", name, it->first);
            return false;
        }

        m_elements.push_back({ name, &ramsesObject });

        return true;
    }

    const RamsesRenderGroupBindingElementsImpl::Elements& RamsesRenderGroupBindingElementsImpl::getElements() const
    {
        return m_elements;
    }
}
