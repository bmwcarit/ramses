//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/Resource.h"
#include "ramses/client/EffectInputSemantic.h"
#include "ramses/framework/AppearanceEnums.h"

#include <optional>
#include <string_view>

namespace ramses
{
    namespace internal
    {
        class EffectImpl;
    }

    class UniformInput;
    class AttributeInput;

    /**
     * @brief An effect describes how an object will be rendered to the screen.
     * @ingroup CoreAPI
    */
    class RAMSES_API Effect : public Resource
    {
    public:
        /**
        * @brief Gets number of uniform inputs.
        *
        * @return Number of uniform inputs
        */
        [[nodiscard]] size_t getUniformInputCount() const;

        /**
        * @brief Gets number of attribute inputs.
        *
        * @return Number of attribute inputs
        */
        [[nodiscard]] size_t getAttributeInputCount() const;

        /**
        * @brief Gets uniform input at given index.
        *
        * @param[in] index Index of uniform input to retrieve
        * @return #ramses::UniformInput if the index is valid, otherwise, std::nullopt
        */
        [[nodiscard]] std::optional<UniformInput> getUniformInput(size_t index) const;

        /**
        * @brief Gets attribute input at given index.
        *
        * @param[in] index Index of attribute input to retrieve
        * @return #ramses::AttributeInput if the index is valid, otherwise, std::nullopt
        */
        [[nodiscard]] std::optional<AttributeInput> getAttributeInput(size_t index) const;

        /**
        * @brief Finds uniform input by input name.
        *
        * @param[in] inputName Name of uniform input to retrieve
        * @return #ramses::UniformInput if successful, otherwise, std::nullopt
        */
        [[nodiscard]] std::optional<UniformInput> findUniformInput(std::string_view inputName) const;

        /**
        * @brief Finds attribute input by input name.
        *
        * @param[in] inputName Name of attribute input to retrieve
        * @return #ramses::AttributeInput if successful, otherwise, std::nullopt
        */
        [[nodiscard]] std::optional<AttributeInput> findAttributeInput(std::string_view inputName) const;

        /**
        * @brief Finds uniform input that represents a semantic input (if existing).
        *
        * @param[in] uniformSemantic Semantic of uniform input to retrieve
        * @return #ramses::UniformInput if successful, otherwise, std::nullopt
        */
        [[nodiscard]] std::optional<UniformInput> findUniformInput(EEffectUniformSemantic uniformSemantic) const;

        /**
        * @brief Finds attribute input that represents a semantic input (if existing).
        *
        * @param[in] attributeSemantic Semantic of attribute input to retrieve
        * @return #ramses::AttributeInput if successful, otherwise, std::nullopt
        */
        [[nodiscard]] std::optional<AttributeInput> findAttributeInput(EEffectAttributeSemantic attributeSemantic) const;

        /**
        * @brief Checks if the \p effect has a geometry shader attached to it.
        *
        * @return true if the effect has a geometry shader attached to it, false otherwise
        */
        [[nodiscard]] bool hasGeometryShader() const;

        /**
        * @brief If the \p effect has a geometry shader attached to it (see #hasGeometryShader) this method
        * can be used to check the expected primitive type of the geometry shader. Use this to make sure
        * that geometry-based effects get only paired with geometry which matches their expected type!
        *
        * See also #ramses::Appearance::setDrawMode().
        *
        * @param[out] expectedGeometryInputType geometry type expected by the geometry shader of /p effect
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool getGeometryShaderInputType(EDrawMode& expectedGeometryInputType) const;

        /**
         * Get the internal data for implementation specifics of Effect.
         */
        [[nodiscard]] internal::EffectImpl& impl();

        /**
         * Get the internal data for implementation specifics of Effect.
         */
        [[nodiscard]] const internal::EffectImpl& impl() const;

    protected:
        /**
        * @brief Scene is the convenience library for application developers
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of Effect
        *
        * @param[in] impl Internal data for implementation specifics of Effect (sink - instance becomes owner)
        */
        explicit Effect(std::unique_ptr<internal::EffectImpl> impl);

        /**
        * @brief Stores internal data for implementation specifics of Effect.
        */
        internal::EffectImpl& m_impl;
    };
}
