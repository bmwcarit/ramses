//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UNIFORMINPUT_H
#define RAMSES_UNIFORMINPUT_H

#include "ramses-client-api/EffectInput.h"
#include "ramses-client-api/EffectInputSemantic.h"
#include "ramses-framework-api/EDataType.h"

namespace ramses
{
    /**
    * @brief The UniformInput is a description of an uniform effect input
    */
    class RAMSES_API UniformInput : public EffectInput
    {
    public:
        /**
        * @brief Constructor of UniformInput.
        */
        UniformInput();

        /**
        * @brief Returns the effect input semantics.
        *
        * @return Effect input semantics
        */
        [[nodiscard]] EEffectUniformSemantic getSemantics() const;

        /**
        * @brief Returns the number of elements that are assigned to this effect input
        * @return the element count or 0 if not initialized
        */
        [[nodiscard]] uint32_t getElementCount() const;
    };
}

#endif
