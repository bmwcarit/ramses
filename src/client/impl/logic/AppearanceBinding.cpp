//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "ramses/client/logic/AppearanceBinding.h"
#include "impl/logic/AppearanceBindingImpl.h"

namespace ramses
{
    AppearanceBinding::AppearanceBinding(std::unique_ptr<internal::AppearanceBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_appearanceBinding{ static_cast<internal::AppearanceBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    ramses::Appearance& AppearanceBinding::getRamsesAppearance() const
    {
        return m_appearanceBinding.getRamsesAppearance();
    }

    internal::AppearanceBindingImpl& AppearanceBinding::impl()
    {
        return m_appearanceBinding;
    }

    const internal::AppearanceBindingImpl& AppearanceBinding::impl() const
    {
        return m_appearanceBinding;
    }
}
