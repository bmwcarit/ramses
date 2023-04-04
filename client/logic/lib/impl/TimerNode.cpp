//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/TimerNode.h"
#include "impl/TimerNodeImpl.h"

namespace rlogic
{
    TimerNode::TimerNode(std::unique_ptr<internal::TimerNodeImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_timerNodeImpl{ static_cast<internal::TimerNodeImpl&>(LogicNode::m_impl) }
    {
    }

    TimerNode::~TimerNode() noexcept = default;
}
