//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/AttributeInput.h"
#include "EffectInputImpl.h"

namespace ramses
{
    AttributeInput::AttributeInput()
        : EffectInput(*(new EffectInputImpl()))
    {
    }

    EEffectInputDataType AttributeInput::getDataType() const
    {
        return impl.getAttributeInputDataType();
    }

    EEffectAttributeSemantic AttributeInput::getSemantics() const
    {
        return impl.getAttributeSemantics();
    }
}
