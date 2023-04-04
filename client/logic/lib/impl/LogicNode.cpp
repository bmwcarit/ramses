//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicNode.h"
#include "impl/LogicNodeImpl.h"

namespace rlogic
{
    LogicNode::LogicNode(std::unique_ptr<internal::LogicNodeImpl> impl) noexcept
        : LogicObject(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_impl{ static_cast<internal::LogicNodeImpl&>(*LogicObject::m_impl) }
    {
    }

    LogicNode::~LogicNode() noexcept = default;

    Property* LogicNode::getInputs()
    {
        return m_impl.getInputs();
    }

    const Property* LogicNode::getInputs() const
    {
        return m_impl.getInputs();
    }

    const Property* LogicNode::getOutputs() const
    {
        return m_impl.getOutputs();
    }
}
