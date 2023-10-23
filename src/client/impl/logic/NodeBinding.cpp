//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/NodeBindingImpl.h"

#include "ramses/client/logic/NodeBinding.h"

namespace ramses
{
    NodeBinding::NodeBinding(std::unique_ptr<internal::NodeBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_nodeBinding{ static_cast<internal::NodeBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    ramses::Node& NodeBinding::getRamsesNode() const
    {
        return m_nodeBinding.getRamsesNode();
    }

    ramses::ERotationType NodeBinding::getRotationType() const
    {
        return m_nodeBinding.getRotationType();
    }

    internal::NodeBindingImpl& NodeBinding::impl()
    {
        return m_nodeBinding;
    }

    const internal::NodeBindingImpl& NodeBinding::impl() const
    {
        return m_nodeBinding;
    }
}
