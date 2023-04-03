//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/SkinBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "impl/SkinBindingImpl.h"
#include "impl/RamsesAppearanceBindingImpl.h"

namespace rlogic
{
    SkinBinding::SkinBinding(std::unique_ptr<internal::SkinBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_skinBinding{ static_cast<internal::SkinBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    SkinBinding::~SkinBinding() noexcept = default;

    const RamsesAppearanceBinding& SkinBinding::getAppearanceBinding() const
    {
        return *m_skinBinding.getAppearanceBinding().getLogicObject().as<RamsesAppearanceBinding>();
    }

    const ramses::UniformInput& SkinBinding::getAppearanceUniformInput() const
    {
        return m_skinBinding.getAppearanceUniformInput();
    }
}
