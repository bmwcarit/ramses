//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/AnchorPoint.h"
#include "impl/logic/AnchorPointImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"

namespace ramses
{
    AnchorPoint::AnchorPoint(std::unique_ptr<internal::AnchorPointImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_anchorPointImpl{ static_cast<internal::AnchorPointImpl&>(LogicNode::m_impl) }
    {
    }

    const Node& AnchorPoint::getRamsesNode() const
    {
        return m_anchorPointImpl.getNodeBinding().getRamsesNode();
    }

    const Camera& AnchorPoint::getRamsesCamera() const
    {
        return m_anchorPointImpl.getCameraBinding().getRamsesCamera();
    }

    internal::AnchorPointImpl& AnchorPoint::impl()
    {
        return m_anchorPointImpl;
    }

    const internal::AnchorPointImpl& AnchorPoint::impl() const
    {
        return m_anchorPointImpl;
    }
}
