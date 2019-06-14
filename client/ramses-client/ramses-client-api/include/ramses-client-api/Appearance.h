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

namespace ramses
{
    class SceneImpl;
    class UniformInput;
    class DataObject;
    class TextureSampler;
    class Effect;

    /**
     * @brief The Appearance describes how an object should look like
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
        * Default draw mode is Triangles.
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
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] i The integer 32bit value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueInt32(const UniformInput& input, int32_t i);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueInt32(const UniformInput& input, uint32_t elementCount, const int32_t* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] i The signed int 32-bit value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueInt32(const UniformInput& input, int32_t& i) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueInt32(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const;


        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] value The float value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueFloat(const UniformInput& input, float value);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueFloat(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] valueOut The float value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueFloat(const UniformInput& input, float& valueOut) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueFloat(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] x The X component value
        * @param[in] y The Y component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector2i(const UniformInput& input, int32_t x, int32_t y);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector2i(const UniformInput& input, uint32_t elementCount, const int32_t* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] x The X component value
        * @param[out] y The Y component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector2i(const UniformInput& input, int32_t& x, int32_t& y) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector2i(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] x The X component value
        * @param[in] y The Y component value
        * @param[in] z The Z component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector3i(const UniformInput& input, int32_t x, int32_t y, int32_t z);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector3i(const UniformInput& input, uint32_t elementCount, const int32_t* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] x The X component value
        * @param[out] y The Y component value
        * @param[out] z The Z component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector3i(const UniformInput& input, int32_t& x, int32_t& y, int32_t& z) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector3i(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] x The X component value
        * @param[in] y The Y component value
        * @param[in] z The Z component value
        * @param[in] w The W component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector4i(const UniformInput& input, int32_t x, int32_t y, int32_t z, int32_t w);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector4i(const UniformInput& input, uint32_t elementCount, const int32_t* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] x The X component value
        * @param[out] y The Y component value
        * @param[out] z The Z component value
        * @param[out] w The W component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector4i(const UniformInput& input, int32_t& x, int32_t& y, int32_t& z, int32_t& w) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector4i(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] x The X component value
        * @param[in] y The Y component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector2f(const UniformInput& input, float x, float y);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector2f(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] x The X component value
        * @param[out] y The Y component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector2f(const UniformInput& input, float& x, float& y) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector2f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] x The X component value
        * @param[in] y The Y component value
        * @param[in] z The Z component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector3f(const UniformInput& input, float x, float y, float z);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector3f(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] x The X component value
        * @param[out] y The Y component value
        * @param[out] z The Z component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector3f(const UniformInput& input, float& x, float& y, float& z) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector3f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] x The X component value
        * @param[in] y The Y component value
        * @param[in] z The Z component value
        * @param[in] w The W component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector4f(const UniformInput& input, float x, float y, float z, float w);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueVector4f(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] x The X component value
        * @param[out] y The Y component value
        * @param[out] z The Z component value
        * @param[out] w The W component value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector4f(const UniformInput& input, float& x, float& y, float& z, float& w) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueVector4f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] values The matrix 2x2 value (column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueMatrix22f(const UniformInput& input, const float values[4]);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values (will be stored column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueMatrix22f(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] valueOut The matrix 2x2 value (column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueMatrix22f(const UniformInput& input, float valueOut[4]) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values (column-wise) are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueMatrix22f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] values The matrix 3x3 value (column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueMatrix33f(const UniformInput& input, const float values[9]);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values (will be stored column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueMatrix33f(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] valueOut The matrix 3x3 value (column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueMatrix33f(const UniformInput& input, float valueOut[9]) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values (column-wise) are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueMatrix33f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

        /**
        * @brief Sets value of the input.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] values The matrix 4x4 value (column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueMatrix44f(const UniformInput& input, const float values[16]);

        /**
        * @brief Sets values of the input elements.
        *
        * @param[in] input The effect uniform input to set the value to
        * @param[in] elementCount The number of values that are used from values. Must match UniformInput::getElementCount()
        * @param[in] values Pointer the the values (will be stored column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputValueMatrix44f(const UniformInput& input, uint32_t elementCount, const float* values);

        /**
        * @brief Gets the value of the input.
        *
        * @param[in] input The effect uniform input
        * @param[out] valueOut The matrix 4x4 value (column-wise)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueMatrix44f(const UniformInput& input, float valueOut[16]) const;

        /**
        * @brief Gets current values of the input.
        *
        * @param[in] input The effect uniform input
        * @param[in] elementCount the number of values that are copied to valuesOut. Must match UniformInput::getElementCount()
        * @param[out] valuesOut location where the values (column-wise) are copied to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getInputValueMatrix44f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const;

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
        * @brief Check whether an uniform input has any DataObject bound to it.
        *
        * @param[in] input The effect uniform input to check
        * @return \c true if there is any DataObject bound to the input, false otherwise
        */
        bool isInputBound(const UniformInput& input) const;

        /**
        * @brief Gets the effect used to create this appearance
        *
        * @return The effect used to create the appearance.
        */
        const Effect& getEffect() const;

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
        virtual ~Appearance();
    };
}

#endif
