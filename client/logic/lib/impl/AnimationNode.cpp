//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/AnimationNode.h"
#include "impl/AnimationNodeImpl.h"

namespace rlogic
{
    AnimationNode::AnimationNode(std::unique_ptr<internal::AnimationNodeImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_animationNodeImpl{ static_cast<internal::AnimationNodeImpl&>(LogicNode::m_impl) }
    {
    }

    AnimationNode::~AnimationNode() noexcept = default;

    const AnimationChannels& AnimationNode::getChannels() const
    {
        return m_animationNodeImpl.getChannels();
    }
}
