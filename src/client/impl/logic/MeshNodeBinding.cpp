//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/MeshNodeBinding.h"
#include "impl/logic/MeshNodeBindingImpl.h"

namespace ramses
{
    MeshNodeBinding::MeshNodeBinding(std::unique_ptr<internal::MeshNodeBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_meshNodeBinding{ static_cast<internal::MeshNodeBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    const ramses::MeshNode& MeshNodeBinding::getRamsesMeshNode() const
    {
        return m_meshNodeBinding.getRamsesMeshNode();
    }

    ramses::MeshNode& MeshNodeBinding::getRamsesMeshNode()
    {
        return m_meshNodeBinding.getRamsesMeshNode();
    }

    internal::MeshNodeBindingImpl& MeshNodeBinding::impl()
    {
        return m_meshNodeBinding;
    }

    const internal::MeshNodeBindingImpl& MeshNodeBinding::impl() const
    {
        return m_meshNodeBinding;
    }
}
