//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECT_H
#define RAMSES_EFFECT_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/EffectInputSemantic.h"
#include "ramses-framework-api/AppearanceEnums.h"

#include <string_view>

namespace ramses
{
    class UniformInput;
    class AttributeInput;

    /**
     * @ingroup CoreAPI
     * @brief An effect describes how an object will be rendered to the screen.
    */
    class Effect : public Resource
    {
    public:
        /**
        * @brief Gets number of uniform inputs.
        *
        * @return Number of uniform inputs
        */
        [[nodiscard]] RAMSES_API size_t getUniformInputCount() const;

        /**
        * @brief Gets number of attribute inputs.
        *
        * @return Number of attribute inputs
        */
        [[nodiscard]] RAMSES_API size_t getAttributeInputCount() const;

        /**
        * @brief Gets uniform input at given index.
        *
        * @param[in] index Index of uniform input to retrieve
        * @param[out] uniformInput Uniform input
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getUniformInput(size_t index, UniformInput& uniformInput) const;

        /**
        * @brief Gets attribute input at given index.
        *
        * @param[in] index Index of attribute input to retrieve
        * @param[out] attributeInput Attribute input
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getAttributeInput(size_t index, AttributeInput& attributeInput) const;

        /**
        * @brief Finds uniform input by input name.
        *
        * @param[in] inputName Name of uniform input to retrieve
        * @param[out] uniformInput Uniform input
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t findUniformInput(std::string_view inputName, UniformInput& uniformInput) const;

        /**
        * @brief Finds attribute input by input name.
        *
        * @param[in] inputName Name of attribute input to retrieve
        * @param[out] attributeInput Attribute input
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t findAttributeInput(std::string_view inputName, AttributeInput& attributeInput) const;

        /**
        * @brief Finds uniform input that represents a semantic input (if existing).
        *
        * @param[in] uniformSemantic Semantic of uniform input to retrieve
        * @param[out] uniformInput Uniform input
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t findUniformInput(EEffectUniformSemantic uniformSemantic, UniformInput& uniformInput) const;

        /**
        * @brief Finds attribute input that represents a semantic input (if existing).
        *
        * @param[in] attributeSemantic Semantic of attribute input to retrieve
        * @param[out] attributeInput Attribute input
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t findAttributeInput(EEffectAttributeSemantic attributeSemantic, AttributeInput& attributeInput) const;

        /**
        * @brief Checks if the \p effect has a geometry shader attached to it.
        *
        * @return true if the effect has a geometry shader attached to it, false otherwise
        */
        [[nodiscard]] RAMSES_API bool hasGeometryShader() const;

        /**
        * @brief If the \p effect has a geometry shader attached to it (see #hasGeometryShader) this method
        * can be used to check the expected primitive type of the geometry shader. Use this to make sure
        * that geometry-based effects get only paired with geometry which matches their expected type!
        *
        * See also #ramses::Appearance::setDrawMode().
        *
        * @param[out] expectedGeometryInputType geometry type expected by the geometry shader of /p effect
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getGeometryShaderInputType(EDrawMode& expectedGeometryInputType) const;

        /**
        * @brief Stores internal data for implementation specifics of Effect.
        */
        class EffectImpl& m_impl;

    protected:
        /**
        * @brief Scene is the convenience library for application developers
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor of Effect
        *
        * @param[in] impl Internal data for implementation specifics of Effect (sink - instance becomes owner)
        */
        explicit Effect(std::unique_ptr<EffectImpl> impl);
    };
}

#endif
