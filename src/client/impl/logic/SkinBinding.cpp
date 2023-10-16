//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"

namespace ramses
{
    SkinBinding::SkinBinding(std::unique_ptr<internal::SkinBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_skinBinding{ static_cast<internal::SkinBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    const AppearanceBinding& SkinBinding::getAppearanceBinding() const
    {
        return *m_skinBinding.getAppearanceBinding().getLogicObject().as<AppearanceBinding>();
    }

    const ramses::UniformInput& SkinBinding::getAppearanceUniformInput() const
    {
        return m_skinBinding.getAppearanceUniformInput();
    }

    internal::SkinBindingImpl& SkinBinding::impl()
    {
        return m_skinBinding;
    }

    const internal::SkinBindingImpl& SkinBinding::impl() const
    {
        return m_skinBinding;
    }
}
