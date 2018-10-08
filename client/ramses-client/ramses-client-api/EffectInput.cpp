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
    EffectInput::EffectInput(EffectInputImpl& effectInputImpl)
        : StatusObject(effectInputImpl)
        , impl(effectInputImpl)
    {
    }

    EffectInput::~EffectInput()
    {
    }

    const char* EffectInput::getName() const
    {
        return impl.getName().c_str();
    }

    bool EffectInput::isValid() const
    {
        return impl.isValid();
    }
}
