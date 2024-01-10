//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/framework/EDataType.h"
#include <optional>
#include <memory>

namespace ramses
{
    namespace internal
    {
        class EffectInputImpl;
    }

    /**
    * @brief The EffectInput is a description of an effect input.
    * @ingroup CoreAPI
    */
    class RAMSES_API EffectInput
    {
    public:
        /**
        * @brief Destructor of EffectInput
        */
        ~EffectInput();

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        EffectInput(const EffectInput& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        EffectInput(EffectInput&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        EffectInput& operator=(const EffectInput& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        EffectInput& operator=(EffectInput&& other) noexcept;

        /**
        * @brief Returns the name of the effect input.
        *
        * @return Name of the effect input
        */
        [[nodiscard]] const char* getName() const;

        /**
        * @brief Returns the effect input data type.
        *
        * @return Effect input data type
        */
        [[nodiscard]] EDataType getDataType() const;

        /**
         * Get the internal data for implementation specifics of EffectInput.
         */
        [[nodiscard]] internal::EffectInputImpl& impl();

        /**
         * Get the internal data for implementation specifics of EffectInput.
         */
        [[nodiscard]] const internal::EffectInputImpl& impl() const;

    protected:
        /**
        * @brief Constructor of EffectInput.
        *
        * @param[in] effectInputImpl Internal data for implementation specifics of EffectInput (sink - instance becomes owner)
        */
        explicit EffectInput(std::unique_ptr<internal::EffectInputImpl> effectInputImpl);

        /**
        * Stores internal data for implementation specifics of EffectInput.
        */
        std::unique_ptr<internal::EffectInputImpl> m_impl;
    };
}
