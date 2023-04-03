//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "impl/RamsesMeshNodeBindingImpl.h"

namespace rlogic
{
    RamsesMeshNodeBinding::RamsesMeshNodeBinding(std::unique_ptr<internal::RamsesMeshNodeBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_meshNodeBinding{ static_cast<internal::RamsesMeshNodeBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    RamsesMeshNodeBinding::~RamsesMeshNodeBinding() noexcept = default;

    const ramses::MeshNode& RamsesMeshNodeBinding::getRamsesMeshNode() const
    {
        return m_meshNodeBinding.getRamsesMeshNode();
    }

    ramses::MeshNode& RamsesMeshNodeBinding::getRamsesMeshNode()
    {
        return m_meshNodeBinding.getRamsesMeshNode();
    }
}
