//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "impl/logic/RenderGroupBindingElementsImpl.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/RenderGroup.h"

namespace ramses
{
    RenderGroupBindingElements::RenderGroupBindingElements() noexcept
        : m_impl{ std::make_unique<internal::RenderGroupBindingElementsImpl>() }
    {
    }
    RenderGroupBindingElements::~RenderGroupBindingElements() noexcept = default;

    RenderGroupBindingElements::RenderGroupBindingElements(const RenderGroupBindingElements& other)
        : m_impl{ std::make_unique<internal::RenderGroupBindingElementsImpl>(*other.m_impl) }
    {
    }

    RenderGroupBindingElements::RenderGroupBindingElements(RenderGroupBindingElements&& other) noexcept = default;

    RenderGroupBindingElements& RenderGroupBindingElements::operator=(const RenderGroupBindingElements& other)
    {
        m_impl = std::make_unique<internal::RenderGroupBindingElementsImpl>(*other.m_impl);
        return *this;
    }

    RenderGroupBindingElements& RenderGroupBindingElements::operator=(RenderGroupBindingElements&& other) noexcept = default;

    bool RenderGroupBindingElements::addElement(const ramses::MeshNode& meshNode, std::string_view elementName)
    {
        return m_impl->addElement(meshNode, elementName);
    }

    bool RenderGroupBindingElements::addElement(const ramses::RenderGroup& nestedRenderGroup, std::string_view elementName)
    {
        return m_impl->addElement(nestedRenderGroup, elementName);
    }

    internal::RenderGroupBindingElementsImpl& RenderGroupBindingElements::impl()
    {
        return *m_impl;
    }

    const internal::RenderGroupBindingElementsImpl& RenderGroupBindingElements::impl() const
    {
        return *m_impl;
    }
}
