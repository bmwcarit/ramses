//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/RamsesRenderGroupBindingElements.h"
#include "impl/RamsesRenderGroupBindingElementsImpl.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RenderGroup.h"

namespace rlogic
{
    RamsesRenderGroupBindingElements::RamsesRenderGroupBindingElements() noexcept
        : m_impl{ std::make_unique<internal::RamsesRenderGroupBindingElementsImpl>() }
    {
    }

    RamsesRenderGroupBindingElements::~RamsesRenderGroupBindingElements() noexcept = default;

    bool RamsesRenderGroupBindingElements::addElement(const ramses::MeshNode& meshNode, std::string_view elementName)
    {
        return m_impl->addElement(meshNode, elementName);
    }

    bool RamsesRenderGroupBindingElements::addElement(const ramses::RenderGroup& nestedRenderGroup, std::string_view elementName)
    {
        return m_impl->addElement(nestedRenderGroup, elementName);
    }
}
