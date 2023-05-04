//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTDESCRIPTION_H
#define RAMSES_EFFECTDESCRIPTION_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-client-api/EffectInputSemantic.h"

namespace ramses
{
    class EffectDescriptionImpl;

    /**
    * @ingroup CoreAPI
    * @brief An effect description holds all necessary information for an effect to be created.
    */
    class EffectDescription : public StatusObject
    {
    public:
        /**
        * @brief Constructor of EffectDescription
        */
        RAMSES_API EffectDescription();
        /**
        * @brief Destructor of EffectDescription
        */
        RAMSES_API ~EffectDescription() override;

        /**
        * @brief Sets vertex shader source from string.
        * @param[in] shaderSource Vertex shader source code.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setVertexShader(const char* shaderSource);
        /**
        * @brief Sets fragment shader source from string.
        * @param[in] shaderSource Fragment shader source code.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setFragmentShader(const char* shaderSource);
        /**
        * @brief Sets geometry shader source from string.
        * @param[in] shaderSource Geometry shader source code.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setGeometryShader(const char* shaderSource);
        /**
        * @brief Reads and sets vertex shader source from file.
        * @param[in] shaderSourceFileName File with vertex shader source code.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setVertexShaderFromFile(const char* shaderSourceFileName);
        /**
        * @brief Reads and sets fragment shader source from file.
        * @param[in] shaderSourceFileName File with fragment shader source code.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setFragmentShaderFromFile(const char* shaderSourceFileName);
        /**
        * @brief Reads and sets geometry shader source from file.
        * @param[in] shaderSourceFileName File with geometry shader source code.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setGeometryShaderFromFile(const char* shaderSourceFileName);
        /**
        * @brief Adds a compiler define.
        *        The define string will be injected as defined into the final shader code.
        * @param[in] define Definition name to be set at compilation time.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t addCompilerDefine(const char* define);

        /**
        * @brief Sets an uniform semantic.
        *        Used for uniforms which are not locally available
        *        on the client, such as projection matrix, framebuffer resolution etc.
        *        Value of an uniform corresponding to the given semantic name
        *        will be automatically set based on its semantic type.
        * @param[in] inputName Name of the effect input as used in the shader source code.
        * @param[in] semanticType Semantic type to be used for given input.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setUniformSemantic(const char* inputName, EEffectUniformSemantic semanticType);

        /**
        * @brief Sets an attribute semantic.
        *        Used to mark attributes as special inputs (eg. text specific inputs).
        *        These attributes are then set to use data provided by Ramses, not user.
        * @param[in] inputName Name of the effect input as used in the shader source code.
        * @param[in] semanticType Semantic type to be used for given input.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setAttributeSemantic(const char* inputName, EEffectAttributeSemantic semanticType);

        /**
        * @brief Gets vertex shader code.
        * @return Vertex shader source code. Empty string if not previously set.
        */
        [[nodiscard]] RAMSES_API const char* getVertexShader() const;
        /**
        * @brief Gets fragment shader code that is currently set.
        * @return Fragment shader source code. Empty string if not previously set.
        */
        [[nodiscard]] RAMSES_API const char* getFragmentShader() const;
        /**
        * @brief Gets geometry shader code that is currently set.
        * @return Geometry shader source code. Empty string if not previously set.
        */
        [[nodiscard]] RAMSES_API const char* getGeometryShader() const;

        /**
        * @brief Gets number of compiler defines.
        * @return Number of compiler defines that were previously added.
        */
        [[nodiscard]] RAMSES_API uint32_t getNumberOfCompilerDefines() const;
        /**
        * @brief Gets compiler define.
        * @param[in] index Index of define to retrieve.
        * @return Compiler define for given index. nullptr if not previously set.
        */
        [[nodiscard]] RAMSES_API const char* getCompilerDefine(uint32_t index) const;

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RAMSES_API EffectDescription(const EffectDescription& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RAMSES_API EffectDescription(EffectDescription&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RAMSES_API EffectDescription& operator=(const EffectDescription& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RAMSES_API EffectDescription& operator=(EffectDescription&& other) noexcept;

        /**
        * @brief Stores internal data for implementation specifics of EffectDescription.
        */
        std::reference_wrapper<EffectDescriptionImpl> m_impl;
    };
}

#endif
