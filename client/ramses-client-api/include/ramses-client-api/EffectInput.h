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
#include "ramses-framework-api/EDataType.h"
#include <optional>

namespace ramses
{
    class EffectInputImpl;

    /**
    * @ingroup CoreAPI
    * @brief The EffectInput is a description of an effect input
    */
    class EffectInput : public StatusObject
    {
    public:
        /**
        * @brief Returns the name of the effect input.
        *
        * @return Name of the effect input
        */
        [[nodiscard]] RAMSES_API const char* getName() const;

        /**
        * @brief Returns the state of the EffectInput object.
        *
        * @return Returns true if this EffectInput object is initialized and refers to an existing effect input
        */
        [[nodiscard]] RAMSES_API bool isValid() const;

        /**
        * @brief Returns the effect input data type.
        *
        * @return Effect input data type if #isValid, std::nullopt otherwise
        */
        [[nodiscard]] RAMSES_API std::optional<EDataType> getDataType() const;

        /**
        * Stores internal data for implementation specifics of EffectInput.
        */
        std::reference_wrapper<EffectInputImpl> m_impl;

    protected:
        /**
        * @brief Constructor of EffectInput.
        *
        * @param[in] effectInputImpl Internal data for implementation specifics of EffectInput (sink - instance becomes owner)
        */
        explicit EffectInput(std::unique_ptr<EffectInputImpl> effectInputImpl);
    };
}

#endif
