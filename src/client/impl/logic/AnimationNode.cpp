//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/AnimationNode.h"
#include "impl/logic/AnimationNodeImpl.h"

namespace ramses
{
    AnimationNode::AnimationNode(std::unique_ptr<internal::AnimationNodeImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_animationNodeImpl{ static_cast<internal::AnimationNodeImpl&>(LogicNode::m_impl) }
    {
    }

    const AnimationChannels& AnimationNode::getChannels() const
    {
        return m_animationNodeImpl.getChannels();
    }

    internal::AnimationNodeImpl& AnimationNode::impl()
    {
        return m_animationNodeImpl;
    }

    const internal::AnimationNodeImpl& AnimationNode::impl() const
    {
        return m_animationNodeImpl;
    }
}
