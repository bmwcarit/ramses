//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ATTRIBUTEINPUT_H
#define RAMSES_ATTRIBUTEINPUT_H

#include "ramses-client-api/EffectInput.h"
#include "ramses-client-api/EffectInputDataType.h"
#include "ramses-client-api/EffectInputSemantic.h"

namespace ramses
{
    /**
    * @brief The AttributeInput is a description of an attribute effect input
    */
    class RAMSES_API AttributeInput : public EffectInput
    {
    public:
        /**
        * @brief Constructor of AttributeInput.
        */
        AttributeInput();

        /**
        * @brief Returns the effect input data type.
        *
        * @return Effect input data type
        */
        [[nodiscard]] EEffectInputDataType getDataType() const;

        /**
        * @brief Returns the effect input semantics.
        *
        * @return Effect input semantics
        */
        [[nodiscard]] EEffectAttributeSemantic getSemantics() const;
    };
}

#endif
