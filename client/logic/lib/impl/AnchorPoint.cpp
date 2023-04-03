//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/AnchorPoint.h"
#include "impl/AnchorPointImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesCameraBindingImpl.h"

namespace rlogic
{
    AnchorPoint::AnchorPoint(std::unique_ptr<internal::AnchorPointImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_anchorPointImpl{ static_cast<internal::AnchorPointImpl&>(LogicNode::m_impl) }
    {
    }

    AnchorPoint::~AnchorPoint() noexcept = default;

    const ramses::Node& AnchorPoint::getRamsesNode() const
    {
        return m_anchorPointImpl.getRamsesNodeBinding().getRamsesNode();
    }

    const ramses::Camera& AnchorPoint::getRamsesCamera() const
    {
        return m_anchorPointImpl.getRamsesCameraBinding().getRamsesCamera();
    }
}
