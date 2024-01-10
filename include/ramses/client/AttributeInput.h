//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/EffectInput.h"
#include "ramses/client/EffectInputSemantic.h"

namespace ramses
{
    namespace internal
    {
        class EffectImpl;
    }

    /**
    * @brief The AttributeInput is a description of an attribute effect input.
    * @ingroup CoreAPI
    */
    class RAMSES_API AttributeInput : public EffectInput
    {
    public:
        /**
        * @brief Returns the effect input semantics.
        *
        * @return Effect input semantics
        */
        [[nodiscard]] EEffectAttributeSemantic getSemantics() const;

        /**
        * @brief Destructor of AttributeInput.
        */
        ~AttributeInput();

        /**
         * @brief Copy constructor.
         * @param other source to copy from
         */
        AttributeInput(const AttributeInput& other);

        /**
         * @brief Move constructor.
         * @param other source to move from
         */
        AttributeInput(AttributeInput&& other) noexcept;


        /**
         * @brief Assignment operator.
         * @param other source to assign from
         */
        AttributeInput& operator=(const AttributeInput& other);

        /**
         * @brief Move assignment operator.
         * @param other source to move assign from
         */
        AttributeInput& operator=(AttributeInput&& other) noexcept;

    protected:
        /**
         * @brief Default constructor of AttributeInput.
         * The default constructor is forbidden from public access. Users are not
         * expected to create AttributeInput objects. Objects can only be created
         * via copy and move constructors, or obtained from #ramses::Effect (see
         * #ramses::Effect::getAttributeInput and #ramses::Effect::findAttributeInput).
         */
        AttributeInput();

    friend class internal::EffectImpl;
    };
}
