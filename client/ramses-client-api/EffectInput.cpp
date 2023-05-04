//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/EffectInput.h"
#include "EffectInputImpl.h"

namespace ramses
{
    EffectInput::EffectInput(std::unique_ptr<EffectInputImpl> effectInputImpl)
        : StatusObject{ std::move(effectInputImpl) }
        , m_impl{ static_cast<EffectInputImpl&>(*StatusObject::m_impl) }
    {
    }

    const char* EffectInput::getName() const
    {
        return m_impl.get().getName().c_str();
    }

    bool EffectInput::isValid() const
    {
        return m_impl.get().isValid();
    }

    std::optional<EDataType> EffectInput::getDataType() const
    {
        return m_impl.get().getDataType();
    }
}
