//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LogicNode.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/logic/LogicNodeImpl.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses
{
    Property::Property(std::unique_ptr<internal::PropertyImpl> impl) noexcept
        : m_impl(std::move(impl))
    {
        m_impl->setPropertyInstance(*this);
    }

    Property::~Property() noexcept = default;

    size_t Property::getChildCount() const
    {
        return m_impl->getChildCount();
    }

    bool Property::hasChild(std::string_view name) const {
        return m_impl->hasChild(name);
    }

    EPropertyType Property::getType() const
    {
        return m_impl->getType();
    }

    std::string_view Property::getName() const
    {
        return m_impl->getName();
    }

    const Property* Property::getChild(size_t index) const
    {
        return m_impl->getChild(index);
    }

    Property* Property::getChild(size_t index)
    {
        return m_impl->getChild(index);
    }

    Property* Property::getChild(std::string_view name)
    {
        return m_impl->getChild(name);
    }

    const Property* Property::getChild(std::string_view name) const
    {
        return m_impl->getChild(name);
    }

    template <typename T> std::optional<T> Property::getInternal() const
    {
        return m_impl->getValue_PublicApi<T>();
    }

    template<typename T>
    bool Property::setInternal(T value)
    {
        return m_impl->setValue_PublicApi(std::move(value));
    }

    // Lua works with int. The logic engine API uses int32_t. To ensure that the runtime has no side effects
    // we assert the two types are equivalent on the platform/compiler
    static_assert(std::is_same<int32_t, int>::value, "int32_t must be the same type as int");

    template RAMSES_API std::optional<float>       Property::getInternal<float>() const;
    template RAMSES_API std::optional<vec2f>       Property::getInternal<vec2f>() const;
    template RAMSES_API std::optional<vec3f>       Property::getInternal<vec3f>() const;
    template RAMSES_API std::optional<vec4f>       Property::getInternal<vec4f>() const;
    template RAMSES_API std::optional<int32_t>     Property::getInternal<int32_t>() const;
    template RAMSES_API std::optional<int64_t>     Property::getInternal<int64_t>() const;
    template RAMSES_API std::optional<vec2i>       Property::getInternal<vec2i>() const;
    template RAMSES_API std::optional<vec3i>       Property::getInternal<vec3i>() const;
    template RAMSES_API std::optional<vec4i>       Property::getInternal<vec4i>() const;
    template RAMSES_API std::optional<std::string> Property::getInternal<std::string>() const;
    template RAMSES_API std::optional<bool>        Property::getInternal<bool>() const;

    template RAMSES_API bool Property::setInternal<float>(float /*value*/);
    template RAMSES_API bool Property::setInternal<vec2f>(vec2f /*value*/);
    template RAMSES_API bool Property::setInternal<vec3f>(vec3f /*value*/);
    template RAMSES_API bool Property::setInternal<vec4f>(vec4f /*value*/);
    template RAMSES_API bool Property::setInternal<int32_t>(int32_t /*value*/);
    template RAMSES_API bool Property::setInternal<int64_t>(int64_t /*value*/);
    template RAMSES_API bool Property::setInternal<vec2i>(vec2i /*value*/);
    template RAMSES_API bool Property::setInternal<vec3i>(vec3i /*value*/);
    template RAMSES_API bool Property::setInternal<vec4i>(vec4i /*value*/);
    template RAMSES_API bool Property::setInternal<std::string>(std::string /*value*/);
    template RAMSES_API bool Property::setInternal<bool>(bool /*value*/);

    bool Property::isLinked() const
    {
        return m_impl->isLinked();
    }

    bool Property::hasIncomingLink() const
    {
        return m_impl->hasIncomingLink();
    }

    bool Property::hasOutgoingLink() const
    {
        return m_impl->hasOutgoingLink();
    }

    std::optional<PropertyLinkConst> Property::getIncomingLink() const
    {
        if (!m_impl->hasIncomingLink())
            return std::nullopt;

        const auto& link = m_impl->getIncomingLink();
        return PropertyLinkConst{ &link.property->getPropertyInstance(), this, link.isWeakLink };
    }

    std::optional<PropertyLink> Property::getIncomingLink()
    {
        if (!m_impl->hasIncomingLink())
            return std::nullopt;

        auto& link = m_impl->getIncomingLink();
        return PropertyLink{ &link.property->getPropertyInstance(), this, link.isWeakLink };
    }

    size_t Property::getOutgoingLinksCount() const
    {
        return m_impl->hasOutgoingLink() ? m_impl->getOutgoingLinks().size() : 0u;
    }

    std::optional<PropertyLinkConst> Property::getOutgoingLink(size_t index) const
    {
        if (index >= getOutgoingLinksCount())
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "Failed to get outgoing link: zero-based index #{} exceeds the total count of outgoing links which is {}.", index, getOutgoingLinksCount());
            return std::nullopt;
        }

        const auto& link = m_impl->getOutgoingLinks()[index];
        return PropertyLinkConst{ this, &link.property->getPropertyInstance(), link.isWeakLink };
    }

    std::optional<PropertyLink> Property::getOutgoingLink(size_t index)
    {
        if (index >= getOutgoingLinksCount())
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "Failed to get outgoing link: zero-based index #{} exceeds the total count of outgoing links which is {}.", index, getOutgoingLinksCount());
            return std::nullopt;
        }

        auto& link = m_impl->getOutgoingLinks()[index];
        return PropertyLink{ this, &link.property->getPropertyInstance(), link.isWeakLink };
    }

    const LogicNode& Property::getOwningLogicNode() const
    {
        return *m_impl->getLogicNode().getLogicObject().as<LogicNode>();
    }

    LogicNode& Property::getOwningLogicNode()
    {
        return *m_impl->getLogicNode().getLogicObject().as<LogicNode>();
    }

    internal::PropertyImpl& Property::impl()
    {
        return *m_impl;
    }

    const internal::PropertyImpl& Property::impl() const
    {
        return *m_impl;
    }
}
