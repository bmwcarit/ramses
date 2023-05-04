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
    * @ingroup CoreAPI
    * @brief The UniformInput is a description of an uniform effect input
    */
    class UniformInput : public EffectInput
    {
    public:
        /**
        * @brief Constructor of UniformInput.
        */
        RAMSES_API UniformInput();

        /**
        * @brief Destructor of UniformInput
        */
        RAMSES_API ~UniformInput() override;

        /**
        * @brief Returns the effect input semantics.
        *
        * @return Effect input semantics
        */
        [[nodiscard]] RAMSES_API EEffectUniformSemantic getSemantics() const;

        /**
        * @brief Returns the number of elements that are assigned to this effect input
        * @return the element count or 0 if not initialized
        */
        [[nodiscard]] RAMSES_API uint32_t getElementCount() const;

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RAMSES_API UniformInput(const UniformInput& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RAMSES_API UniformInput(UniformInput&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RAMSES_API UniformInput& operator=(const UniformInput& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RAMSES_API UniformInput& operator=(UniformInput&& other) noexcept;
    };
}

#endif
