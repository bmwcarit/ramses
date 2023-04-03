//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APPEARANCE_H
#define RAMSES_APPEARANCE_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-client-api/AppearanceEnums.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    class SceneImpl;
    class UniformInput;
    class DataObject;
    class TextureSampler;
    class TextureSamplerMS;
    class TextureSamplerExternal;
    class Effect;

    /**
     * @brief The Appearance describes how an object should look like. This includes GLSL uniform values,
     * and GPU states such as blending, buffer configurations, masks etc. The API to set uniform values
     * is aligned to the glUniformX API of OpenGL. Beware that boolean values are reported and handled
     * as int (0 is false, anything else is true) - similar to OpenGL conventions.
     *
     * It provides mechanisms for creating effects
    */
    class RAMSES_API Appearance : public SceneObject
    {
    public:
        /**
        * @brief Sets blending factors for source/destination color/alpha.
        * Blending operations need to be set as well in order to enable blending.
        *
        * @param[in] srcColor Source color blending factor
        * @param[in] destColor Destination color blending factor
        * @param[in] srcAlpha Source alpha blending factor
        * @param[in] destAlpha Destination alpha blending factor
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha);

        /**
        * @brief Gets blending factors for source/destination color/alpha.
        *
        * @param[out] srcColor Source color blending factor
        * @param[out] destColor Destination color blending factor
        * @param[out] srcAlpha Source alpha blending factor
        * @param[out] destAlpha Destination alpha blending factor
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const;

        /**
        * @brief Sets blending operation for color and alpha.
        * Blending factors need to be set as well in order to enable blending.
        *
        * @param[in] operationColor Blending operation for color
        * @param[in] operationAlpha Blending operation for alpha
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha);

        /**
        * @brief Gets blending operation for color and alpha.
        *
        * @param[out] operationColor Blending operation for color
        * @param[out] operationAlpha Blending operation for alpha
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const;

        /**
        * @brief Sets blending color that can be used as blending color constant for some blending factors.
        *        The default value is (0,0,0,0)
        *
        * @param[in] color RGBA channels of blending color to set ([0;1] range)
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setBlendingColor(const vec4f& color);

        /**
        * @brief Gets blending color set via #setBlendingColor
        *
        * @param[out] color RGBA channels of blending color ([0;1] range)
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getBlendingColor(vec4f& color) const;

        /**
        * @brief Enables or disables writing to depth buffer.
        *
        * @param[in] mode Flag denoting enabling or disabling depth writes.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setDepthWrite(EDepthWrite mode);

        /**
        * @brief Gets the current state of writing to depth buffer.
        * @param[out] mode Depth write mode
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getDepthWriteMode(EDepthWrite& mode) const;

        /**
        * @brief Sets depth comparison function.
        * Depth writing has to be enabled in order for this to have any effect.
        * Default depth comparison function is less or equal.
        *
        * @param[in] func Depth comparison function to be used
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setDepthFunction(EDepthFunc func);

        /**
        * @brief Gets depth comparison function.
        *
        * @param[out] func Depth comparison function to be used
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getDepthFunction(EDepthFunc& func) const;

        /**
        * @brief Enables or disables scissor test and sets region for scissor test
        *
        * @param[in] state Flag denoting enabling or disabling scissor test.
        * @param[in] x Offset of scissor region on x-axis.
        * @param[in] y Offset of scissor region on y-axis.
        * @param[in] width Width of scissor region.
        * @param[in] height Height of scissor region.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setScissorTest(EScissorTest state, int16_t x, int16_t y, uint16_t width, uint16_t height);

        /**
        * @brief Gets the current state of scissor test.
        * @param[out] state State of scissor test
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getScissorTestState(EScissorTest& state) const;

        /**
        * @brief Gets region for scissor test
        *
        * @param[out] x Offset of scissor region on x-axis.
        * @param[out] y Offset of scissor region on y-axis.
        * @param[out] width Width of scissor region.
        * @param[out] height Height of scissor region.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const;

        /**
        * @brief Sets stencil function, reference and mask value for stencil testing.
        * Stencil is disabled by default.
        *
        * @param[in] func Stencil function to be used
        * @param[in] ref Stencil reference value to be used
        * @param[in] mask Stencil mask value to be used
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setStencilFunction(EStencilFunc func, uint8_t ref, uint8_t mask);

        /**
        * @brief Gets stencil function, reference and mask value
        *
        * @param[out] func Stencil function currently set
        * @param[out] ref Stencil reference value currently set
        * @param[out] mask Stencil mask value currently set
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getStencilFunction(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const;

        /**
        * @brief Sets stencil operations for stencil testing.
        * Default stencil operation values are keep.
        *
        * @param[in] sfail Stencil operation when stencil test fails
        * @param[in] dpfail Stencil operation when the stencil test passes, but the depth test fails
        * @param[in] dppass Stencil operation when both the stencil test and the depth test pass
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass);

        /**
        * @brief Gets stencil operations
        *
        * @param[out] sfail Stencil operation when stencil test fails
        * @param[out] dpfail Stencil operation when the stencil test passes, but the depth test fails
        * @param[out] dppass Stencil operation when both the stencil test and the depth test pass
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const;

        /**
        * @brief Sets the culling mode indicating which side of mesh will be removed before rasterization.
        * Default culling mode is BackFaceCulling.
        *
        * @param[in] mode Culling mode to be used.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setCullingMode(ECullMode mode);

        /**
        * @brief Sets the draw mode indicating by which primitive the mesh will be rendered
        * Default draw mode is #ramses::EDrawMode_Triangles, however if the effect used
        * has a geometry shader, then the input type declared in the geometry shader is set
        * as draw mode automatically when #Appearance is created.
        *
        * @param[in] mode Draw mode to be used.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setDrawMode(EDrawMode mode);

        /**
        * @brief Gets the culling mode indicating which side of mesh will be removed before rasterization.
        *
        * @param[out] mode Culling mode to be used.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getCullingMode(ECullMode& mode) const;

        /**
        * @brief Gets the draw mode indicating by which primitive the mesh will be rendered
        *
        * @param[out] mode draw mode to be used.
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getDrawMode(EDrawMode& mode) const;

        /**
        * @brief Sets color write mask.
        * If needed certain color channels can stay untouched using the color write mask.
        * By default writing to all color channels is enabled.
        *
        * @param[in] writeRed Enable/disable flag for red channel
        * @param[in] writeGreen Enable/disable flag for green channel
        * @param[in] writeBlue Enable/disable flag for blue channel
        * @param[in] writeAlpha Enable/disable flag for alpha channel
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha);

        /**
        * @brief Gets color write mask.
        *
        * @param[out] writeRed Enable/disable flag for red channel
        * @param[out] writeGreen Enable/disable flag for green channel
        * @param[out] writeBlue Enable/disable flag for blue channel
        * @param[out] writeAlpha Enable/disable flag for alpha channel
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const;

        /**
        * Stores internal data for implementation specifics of Appearance.
        */
        class AppearanceImpl& impl;

        /**
        * @brief Sets value to uniform input.
        * Value type must pass #ramses::IsUniformInputDataType.
        * This method will fail if value type not compatible with the uniform data type
        * (see #ramses::UniformInput::getDataType).
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] value The value to set
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        template <typename T>
        status_t setInputValue(const UniformInput& input, T&& value);

        /**
        * @brief Sets value(s) to uniform input.
        * Value type must pass #ramses::IsUniformInputDataType.
        * This method will fail if value type not compatible with the uniform data type
        * (see #ramses::UniformInput::getDataType).
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of elements to use from \c values.
        *                         Must match #ramses::UniformInput::getElementCount.
        * @param[in] values Pointer the the values, must contain at least \c elementCount elements.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        template <typename T>
        status_t setInputValue(const UniformInput& input, uint32_t elementCount, const T* values);

        /**
        * @brief Gets value of uniform input.
        * Value type must pass #ramses::IsUniformInputDataType.
        * This method will fail if value type not compatible with the uniform data type
        * (see #ramses::UniformInput::getDataType).
        *
        * @param[in] input The effect uniform input
        * @param[out] value The value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        template <typename T>
        status_t getInputValue(const UniformInput& input, T& value) const;

        /**
        * @brief Gets value(s) of uniform input.
        * Value type must pass #ramses::IsUniformInputDataType.
        * This method will fail if value type not compatible with the uniform data type
        * (see #ramses::UniformInput::getDataType).
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount The number of elements to copy to \c valuesOut.
        *                         Must match #ramses::UniformInput::getElementCount.
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        template <typename T>
        status_t getInputValue(const UniformInput& input, uint32_t elementCount, T* valuesOut) const;

        /**
        * @brief Sets texture sampler to the input
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] textureSampler The texture sampler
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputTexture(const UniformInput& input, const TextureSampler& textureSampler);

        /**
        * @brief Sets multisampled texture sampler to the input
        *
        * @param[in] input The multisampled texture sampler uniform input to set the value to
        * @param[in] textureSampler The multisampled texture sampler
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputTexture(const UniformInput& input, const TextureSamplerMS& textureSampler);

        /**
        * @brief Sets external texture sampler to the input
        *
        * @param[in] input The external texture sampler uniform input to set the value to
        * @param[in] textureSampler The external texture sampler
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputTexture(const UniformInput& input, const TextureSamplerExternal& textureSampler);

        /**
        * @brief Gets texture sampler currently set to the input
        *
        * @param[in] input The effect uniform input
        * @param[out] textureSampler Will set texture sampler pointer to the TextureSampler object set to the uniform input,
        *                            nullptr if none set or there was an error
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputTexture(const UniformInput& input, const TextureSampler*& textureSampler) const;

        /**
        * @brief Gets texture sampler currently set to the input
        *
        * @param[in] input The effect uniform input
        * @param[out] textureSampler Will set texture sampler pointer to the TextureSamplerMS object set to the uniform input,
        *                            nullptr if none set or there was an error
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputTextureMS(const UniformInput& input, const TextureSamplerMS*& textureSampler) const;

        /**
        * @brief Gets texture sampler currently set to the input
        *
        * @param[in] input The effect uniform input
        * @param[out] textureSampler Will set texture sampler pointer to the TextureSamplerExternal object set to the uniform input,
        *                            nullptr if none set or there was an error
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputTextureExternal(const UniformInput& input, const TextureSamplerExternal*& textureSampler) const;

        /**
        * @brief Bind a DataObject to the Appearance's uniform input.
        *        The value from the DataObject will be used and any change made on the DataObject
        *        will be reflected in the Appearance. One DataObject can be bound to multiple Appearances.
        *        The data type of the DataObject must match the uniform input data type otherwise
        *        the call will fail and report error.
        *        DataObject cannot be bound to an input with semantics, texture input or array input.
        *        Once a DataObject is bound to an input the value cannot be set or get using \c set/getInputValue*() anymore.
        *        Binding a DataObject to an already bound input will unbind the old one and bind the new one.
        *
        * @param[in] input The effect uniform input to bind the DataObject to
        * @param[in] dataObject The DataObject to be bound
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t bindInput(const UniformInput& input, const DataObject& dataObject);

        /**
        * @brief Unbind a previously bound DataObject from the Appearance's uniform input.
        *        Any previously set value that was set before binding will now be used.
        *        Appropriate \c set/getInputValue*() method must be used to set or get the value
        *        or another DataObject can be bound.
        *
        * @param[in] input The effect uniform input to unbind the DataObject from
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unbindInput(const UniformInput& input);

        /**
        * @brief Check whether a uniform input has any DataObject bound to it.
        *
        * @param[in] input The effect uniform input to check
        * @return \c true if there is any DataObject bound to the input, false otherwise
        */
        [[nodiscard]] bool isInputBound(const UniformInput& input) const;

        /**
        * @brief Gets the data object bound to a uniform input.
        *
        * @param[in] input The effect uniform input to get the bound data object for
        * @return \c The data object bound the uniform input if existing, otherwise returns nullptr
        */
        [[nodiscard]] const DataObject* getDataObjectBoundToInput(const UniformInput& input) const;

        /**
        * @brief Gets the effect used to create this appearance
        *
        * @return The effect used to create the appearance.
        */
        [[nodiscard]] const Effect& getEffect() const;

    protected:
        /**
        * @brief Scene is the factory for creating Appearance instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of Appearance
        *
        * @param[in] pimpl Internal data for implementation specifics of Appearance (sink - instance becomes owner)
        */
        explicit Appearance(AppearanceImpl& pimpl);

        /**
        * @brief Copy constructor of Appearance
        *
        * @param[in] other Other instance of appearance class
        */
        Appearance(const Appearance& other);

        /**
        * @brief Assignment operator of Appearance.
        *
        * @param[in] other Other instance of appearance class
        * @return This instance after assignment
        */
        Appearance& operator=(const Appearance& other);

        /**
        * @brief Destructor of the Appearance
        */
        ~Appearance() override;

    private:
        /// Internal implementation of #setInputValue
        template <typename T> RAMSES_API status_t setInputValueInternal(const UniformInput& input, T&& value);
        /// Internal implementation of #setInputValue
        template <typename T> RAMSES_API status_t setInputValueInternal(const UniformInput& input, uint32_t elementCount, const T* values);
        /// Internal implementation of #getInputValue
        template <typename T> RAMSES_API status_t getInputValueInternal(const UniformInput& input, T& value) const;
        /// Internal implementation of #getInputValue
        template <typename T> RAMSES_API status_t getInputValueInternal(const UniformInput& input, uint32_t elementCount, T* valuesOut) const;
    };

    template <typename T> status_t Appearance::setInputValue(const UniformInput& input, T&& value)
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported uniform data type!");
        return setInputValueInternal(input, std::forward<T>(value));
    }

    template <typename T> status_t Appearance::setInputValue(const UniformInput& input, uint32_t elementCount, const T* values)
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported uniform data type!");
        return setInputValueInternal<T>(input, elementCount, values);
    }

    template <typename T> status_t Appearance::getInputValue(const UniformInput& input, T& value) const
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported uniform data type!");
        return getInputValueInternal<T>(input, value);
    }

    template <typename T> status_t Appearance::getInputValue(const UniformInput& input, uint32_t elementCount, T* valuesOut) const
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported uniform data type!");
        return getInputValueInternal<T>(input, elementCount, valuesOut);
    }
}

#endif
