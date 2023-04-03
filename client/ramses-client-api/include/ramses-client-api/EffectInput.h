//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTINPUT_H
#define RAMSES_EFFECTINPUT_H

#include "ramses-framework-api/StatusObject.h"

namespace ramses
{
    /**
    * @brief The EffectInput is a description of an effect input
    */
    class RAMSES_API EffectInput : public StatusObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of EffectInput.
        */
        class EffectInputImpl& impl;

        /**
        * @brief Destructor of EffectInput
        */
        ~EffectInput() override;

        /**
        * @brief Returns the name of the effect input.
        *
        * @return Name of the effect input
        */
        [[nodiscard]] const char* getName() const;

        /**
        * @brief Returns the state of the EffectInput object.
        *
        * @return Returns true if this EffectInput object is initialized and refers to an existing effect input
        */
        [[nodiscard]] bool isValid() const;

    protected:
        /**
        * @brief Constructor of EffectInput.
        *
        * @param[in] effectInputImpl Internal data for implementation specifics of EffectInput (sink - instance becomes owner)
        */
        explicit EffectInput(EffectInputImpl& effectInputImpl);
    };
}

#endif
