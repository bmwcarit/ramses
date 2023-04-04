//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/UniformInput.h"
#include "EffectInputImpl.h"
#include "EffectInputSemanticUtils.h"

namespace ramses
{
    UniformInput::UniformInput()
        : EffectInput(*(new EffectInputImpl()))
    {
    }

    EEffectInputDataType UniformInput::getDataType() const
    {
        return impl.getUniformInputDataType();
    }

    EEffectUniformSemantic UniformInput::getSemantics() const
    {
        return impl.getUniformSemantics();
    }

    uint32_t UniformInput::getElementCount() const
    {
        return impl.getElementCount();
    }
}
