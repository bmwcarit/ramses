//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesNodeBindingImpl.h"

#include "ramses-logic/RamsesNodeBinding.h"

namespace rlogic
{
    RamsesNodeBinding::RamsesNodeBinding(std::unique_ptr<internal::RamsesNodeBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_nodeBinding{ static_cast<internal::RamsesNodeBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    RamsesNodeBinding::~RamsesNodeBinding() noexcept = default;

    ramses::Node& RamsesNodeBinding::getRamsesNode() const
    {
        return m_nodeBinding.getRamsesNode();
    }

    ERotationType RamsesNodeBinding::getRotationType() const
    {
        return m_nodeBinding.getRotationType();
    }
}
