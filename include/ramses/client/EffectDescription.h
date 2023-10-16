//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/client/EffectInputSemantic.h"

#include <string_view>
#include <memory>

namespace ramses
{
    namespace internal
    {
        class EffectDescriptionImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief An effect description holds all necessary information for an effect to be created.
    */
    class RAMSES_API EffectDescription
    {
    public:
        /**
        * @brief Constructor of EffectDescription
        */
        EffectDescription();
        /**
        * @brief Destructor of EffectDescription
        */
        ~EffectDescription();

        /**
        * @brief Sets vertex shader source from string.
        * @param[in] shaderSource Vertex shader source code.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setVertexShader(std::string_view shaderSource);
        /**
        * @brief Sets fragment shader source from string.
        * @param[in] shaderSource Fragment shader source code.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setFragmentShader(std::string_view shaderSource);
        /**
        * @brief Sets geometry shader source from string.
        * @param[in] shaderSource Geometry shader source code.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setGeometryShader(std::string_view shaderSource);
        /**
        * @brief Reads and sets vertex shader source from file.
        * @param[in] shaderSourceFileName File with vertex shader source code.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setVertexShaderFromFile(std::string_view shaderSourceFileName);
        /**
        * @brief Reads and sets fragment shader source from file.
        * @param[in] shaderSourceFileName File with fragment shader source code.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setFragmentShaderFromFile(std::string_view shaderSourceFileName);
        /**
        * @brief Reads and sets geometry shader source from file.
        * @param[in] shaderSourceFileName File with geometry shader source code.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setGeometryShaderFromFile(std::string_view shaderSourceFileName);
        /**
        * @brief Adds a compiler define.
        *        The define string will be injected as defined into the final shader code.
        * @param[in] define Definition name to be set at compilation time.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool addCompilerDefine(std::string_view define);

        /**
        * @brief Sets an uniform semantic.
        *        Used for uniforms which are not locally available
        *        on the client, such as projection matrix, framebuffer resolution etc.
        *        Value of an uniform corresponding to the given semantic name
        *        will be automatically set based on its semantic type.
        * @param[in] inputName Name of the effect input as used in the shader source code.
        * @param[in] semanticType Semantic type to be used for given input.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setUniformSemantic(std::string_view inputName, EEffectUniformSemantic semanticType);

        /**
        * @brief Sets an attribute semantic.
        *        Used to mark attributes as special inputs (eg. text specific inputs).
        *        These attributes are then set to use data provided by Ramses, not user.
        * @param[in] inputName Name of the effect input as used in the shader source code.
        * @param[in] semanticType Semantic type to be used for given input.
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setAttributeSemantic(std::string_view inputName, EEffectAttributeSemantic semanticType);

        /**
        * @brief Gets vertex shader code.
        * @return Vertex shader source code. Empty string if not previously set.
        */
        [[nodiscard]] const char* getVertexShader() const;
        /**
        * @brief Gets fragment shader code that is currently set.
        * @return Fragment shader source code. Empty string if not previously set.
        */
        [[nodiscard]] const char* getFragmentShader() const;
        /**
        * @brief Gets geometry shader code that is currently set.
        * @return Geometry shader source code. Empty string if not previously set.
        */
        [[nodiscard]] const char* getGeometryShader() const;

        /**
        * @brief Gets number of compiler defines.
        * @return Number of compiler defines that were previously added.
        */
        [[nodiscard]] size_t getNumberOfCompilerDefines() const;
        /**
        * @brief Gets compiler define.
        * @param[in] index Index of define to retrieve.
        * @return Compiler define for given index. nullptr if not previously set.
        */
        [[nodiscard]] const char* getCompilerDefine(size_t index) const;

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        EffectDescription(const EffectDescription& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        EffectDescription(EffectDescription&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        EffectDescription& operator=(const EffectDescription& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        EffectDescription& operator=(EffectDescription&& other) noexcept;

        /**
         * Get the internal data for implementation specifics of EffectDescription.
         */
        [[nodiscard]] internal::EffectDescriptionImpl& impl();

        /**
         * Get the internal data for implementation specifics of EffectDescription.
         */
        [[nodiscard]] const internal::EffectDescriptionImpl& impl() const;

    protected:
        /**
        * @brief Stores internal data for implementation specifics of EffectDescription.
        */
        std::unique_ptr<internal::EffectDescriptionImpl> m_impl;
    };
}
