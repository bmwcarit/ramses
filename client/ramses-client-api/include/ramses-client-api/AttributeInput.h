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
#include "ramses-client-api/EffectInputSemantic.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief The AttributeInput is a description of an attribute effect input
    */
    class AttributeInput : public EffectInput
    {
    public:
        /**
        * @brief Constructor of AttributeInput.
        */
        RAMSES_API AttributeInput();

        /**
        * @brief Destructor of AttributeInput.
        */
        RAMSES_API ~AttributeInput() override;

        /**
        * @brief Returns the effect input semantics.
        *
        * @return Effect input semantics
        */
        [[nodiscard]] RAMSES_API EEffectAttributeSemantic getSemantics() const;

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RAMSES_API AttributeInput(const AttributeInput& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RAMSES_API AttributeInput(AttributeInput&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RAMSES_API AttributeInput& operator=(const AttributeInput& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RAMSES_API AttributeInput& operator=(AttributeInput&& other) noexcept;
    };
}

#endif
