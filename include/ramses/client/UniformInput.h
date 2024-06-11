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
    * @brief The UniformInput is a description of an uniform effect input.
    * @ingroup CoreAPI
    */
    class RAMSES_API UniformInput : public EffectInput
    {
    public:
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
        [[nodiscard]] size_t getElementCount() const;

        /**
        * @brief Returns the binding specified in the layout for uniform buffer declaration in the shader.
        * @details If the uniform object input reports to either a uniform buffer or a uniform buffer field/item,
        *          then the binding specified for the uniform buffer in layout is returned. Same value is returned
        *          for inputs representing a uniform buffer and all its fields.
        * @return the uniform buffer binding or nullopt if the input is neither a uniform buffer nor a field within a uniform buffer
        */
        [[nodiscard]] std::optional<uint32_t> getUniformBufferBinding() const;

        /**
        * @brief Destructor of UniformInput.
        */
        ~UniformInput();

        /**
         * @brief Copy constructor.
         * @param other source to copy from
         */
        UniformInput(const UniformInput& other);

        /**
         * @brief Move constructor.
         * @param other source to move from
         */
        UniformInput(UniformInput&& other) noexcept;

        /**
         * @brief Assignment operator.
         * @param other source to assign from
         */
        UniformInput& operator=(const UniformInput& other);

        /**
         * @brief Move assignment operator.
         * @param other source to move assign from
         */
        UniformInput& operator=(UniformInput&& other) noexcept;

    protected:
        /**
         * @brief Default constructor of UniformInput.
         * The default constructor is forbidden from public access. Users are not
         * expected to create UniformInput objects. Objects can only be created
         * via copy and move constructors, or obtained from #ramses::Effect (see
         * #ramses::Effect::getUniformInput and #ramses::Effect::findUniformInput).
         */
        UniformInput();

        friend class internal::EffectImpl;
    };
}
