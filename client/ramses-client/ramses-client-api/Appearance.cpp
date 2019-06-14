//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/DataObject.h"

// internal
#include "AppearanceImpl.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"

namespace ramses
{
    Appearance::Appearance(AppearanceImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    Appearance::~Appearance()
    {
    }

    status_t Appearance::setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        const status_t status = impl.setBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        LOG_HL_CLIENT_API4(status, srcColor, destColor, srcAlpha, destAlpha);
        return status;
    }

    status_t Appearance::getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const
    {
        return impl.getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
    }

    status_t Appearance::setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        const status_t status = impl.setBlendingOperations(operationColor, operationAlpha);
        LOG_HL_CLIENT_API2(status, operationColor, operationAlpha);
        return status;
    }

    status_t Appearance::getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const
    {
        return impl.getBlendingOperations(operationColor, operationAlpha);
    }

    status_t Appearance::setDepthWrite(EDepthWrite mode)
    {
        const status_t status = impl.setDepthWrite(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    status_t Appearance::getDepthWriteMode(EDepthWrite& mode) const
    {
        return impl.getDepthWriteMode(mode);
    }

    status_t Appearance::setDepthFunction(EDepthFunc func)
    {
        const status_t status = impl.setDepthFunction(func);
        LOG_HL_CLIENT_API1(status, func);
        return status;
    }

    status_t Appearance::getDepthFunction(EDepthFunc& func) const
    {
        return impl.getDepthFunction(func);
    }

    status_t Appearance::setScissorTest(EScissorTest mode, int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        const status_t status = impl.setScissorTest(mode, x, y, width, height);
        LOG_HL_CLIENT_API5(status, mode, x, y, width, height);
        return status;
    }

    status_t Appearance::getScissorTestState(EScissorTest& state) const
    {
        return impl.getScissorTestState(state);
    }

    status_t Appearance::getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const
    {
        return impl.getScissorRegion(x, y, width, height);
    }

    status_t Appearance::setStencilFunction(EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        const status_t status = impl.setStencilFunc(func, ref, mask);
        LOG_HL_CLIENT_API3(status, func, ref, mask);
        return status;
    }

    status_t Appearance::getStencilFunction(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const
    {
        return impl.getStencilFunc(func, ref, mask);
    }

    status_t Appearance::setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass)
    {
        const status_t status = impl.setStencilOperation(sfail, dpfail, dppass);
        LOG_HL_CLIENT_API3(status, sfail, dpfail, dppass);
        return status;
    }

    status_t Appearance::getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const
    {
        return impl.getStencilOperation(sfail, dpfail, dppass);
    }

    status_t Appearance::setCullingMode(ECullMode mode)
    {
        const status_t status = impl.setCullingMode(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    status_t Appearance::getCullingMode(ECullMode& mode) const
    {
        return impl.getCullingMode(mode);
    }

    status_t Appearance::setDrawMode(EDrawMode mode)
    {
        const status_t status = impl.setDrawMode(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    status_t Appearance::getDrawMode(EDrawMode& mode) const
    {
        return impl.getDrawMode(mode);
    }

    status_t Appearance::setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
    {
        const status_t status = impl.setColorWriteMask(writeRed, writeGreen, writeBlue, writeAlpha);
        LOG_HL_CLIENT_API4(status, writeRed, writeGreen, writeBlue, writeAlpha);
        return status;
    }

    status_t Appearance::getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const
    {
        return impl.getColorWriteMask(writeRed, writeGreen, writeBlue, writeAlpha);
    }

    status_t Appearance::setInputValueInt32(const UniformInput& input, int32_t i)
    {
        return impl.setInputValue(input.impl, 1u, &i);
    }

    status_t Appearance::setInputValueInt32(const UniformInput& input, uint32_t elementCount, const int32_t* values)
    {
        return impl.setInputValue(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueInt32(const UniformInput& input, int32_t& i) const
    {
        return impl.getInputValue(input.impl, 1u, &i);
    }

    status_t Appearance::getInputValueInt32(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const
    {
        return impl.getInputValue(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueFloat(const UniformInput& input, float x)
    {
        return impl.setInputValue(input.impl, 1u, &x);
    }

    status_t Appearance::setInputValueFloat(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValue(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueFloat(const UniformInput& input, float& x) const
    {
        return impl.getInputValue(input.impl, 1u, &x);
    }

    status_t Appearance::getInputValueFloat(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValue(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueVector2i(const UniformInput& input, int32_t x, int32_t y)
    {
        ramses_internal::Vector2i vecVal(x, y);
        return impl.setInputValue(input.impl, 1u, &vecVal);
    }

    status_t Appearance::setInputValueVector2i(const UniformInput& input, uint32_t elementCount, const int32_t* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Vector2i>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueVector2i(const UniformInput& input, int32_t& x, int32_t& y) const
    {
        ramses_internal::Vector2i vecVal;
        const status_t stat = impl.getInputValue(input.impl, 1u, &vecVal);
        x = vecVal.x;
        y = vecVal.y;
        return stat;
    }

    status_t Appearance::getInputValueVector2i(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Vector2i>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueVector3i(const UniformInput& input, int32_t x, int32_t y, int32_t z)
    {
        ramses_internal::Vector3i vecVal(x, y, z);
        return impl.setInputValue(input.impl, 1u, &vecVal);
    }

    status_t Appearance::setInputValueVector3i(const UniformInput& input, uint32_t elementCount, const int32_t* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Vector3i>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueVector3i(const UniformInput& input, int32_t& x, int32_t& y, int32_t& z) const
    {
        ramses_internal::Vector3i vecVal;
        const status_t stat = impl.getInputValue(input.impl, 1u, &vecVal);
        x = vecVal.x;
        y = vecVal.y;
        z = vecVal.z;
        return stat;
    }

    status_t Appearance::getInputValueVector3i(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Vector3i>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueVector4i(const UniformInput& input, int32_t x, int32_t y, int32_t z, int32_t w)
    {
        ramses_internal::Vector4i vecVal(x, y, z, w);
        return impl.setInputValue(input.impl, 1u, &vecVal);
    }

    status_t Appearance::setInputValueVector4i(const UniformInput& input, uint32_t elementCount, const int32_t* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Vector4i>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueVector4i(const UniformInput& input, int32_t& x, int32_t& y, int32_t& z, int32_t& w) const
    {
        ramses_internal::Vector4i vecVal;
        const status_t stat = impl.getInputValue(input.impl, 1u, &vecVal);
        x = vecVal.x;
        y = vecVal.y;
        z = vecVal.z;
        w = vecVal.w;
        return stat;
    }

    status_t Appearance::getInputValueVector4i(const UniformInput& input, uint32_t elementCount, int32_t* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Vector4i>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueVector2f(const UniformInput& input, float x, float y)
    {
        ramses_internal::Vector2 vecVal(x, y);
        return impl.setInputValue(input.impl, 1u, &vecVal);
    }

    status_t Appearance::setInputValueVector2f(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Vector2>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueVector2f(const UniformInput& input, float& x, float& y) const
    {
        ramses_internal::Vector2 vecVal;
        const status_t stat = impl.getInputValue(input.impl, 1u, &vecVal);
        x = vecVal.x;
        y = vecVal.y;
        return stat;
    }

    status_t Appearance::getInputValueVector2f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Vector2>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueVector3f(const UniformInput& input, float x, float y, float z)
    {
        ramses_internal::Vector3 vecVal(x, y, z);
        return impl.setInputValue(input.impl, 1u, &vecVal);
    }

    status_t Appearance::setInputValueVector3f(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Vector3>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueVector3f(const UniformInput& input, float& x, float& y, float& z) const
    {
        ramses_internal::Vector3 vecVal;
        const status_t stat = impl.getInputValue(input.impl, 1u, &vecVal);
        x = vecVal.x;
        y = vecVal.y;
        z = vecVal.z;
        return stat;
    }

    status_t Appearance::getInputValueVector3f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Vector3>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueVector4f(const UniformInput& input, float x, float y, float z, float w)
    {
        ramses_internal::Vector4 vecVal(x, y, z, w);
        return impl.setInputValue(input.impl, 1u, &vecVal);
    }

    status_t Appearance::setInputValueVector4f(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Vector4>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueVector4f(const UniformInput& input, float& x, float& y, float& z, float& w) const
    {
        ramses_internal::Vector4 vecVal;
        const status_t stat = impl.getInputValue(input.impl, 1u, &vecVal);
        x = vecVal.x;
        y = vecVal.y;
        z = vecVal.z;
        w = vecVal.w;
        return stat;
    }

    status_t Appearance::getInputValueVector4f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Vector4>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueMatrix22f(const UniformInput& input, const float values[4])
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix22f>(input.impl, 1u, values);
    }

    status_t Appearance::setInputValueMatrix22f(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix22f>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueMatrix22f(const UniformInput& input, float valueOut[4]) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix22f>(input.impl, 1u, valueOut);
    }

    status_t Appearance::getInputValueMatrix22f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix22f>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueMatrix33f(const UniformInput& input, const float values[9])
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix33f>(input.impl, 1u, values);
    }

    status_t Appearance::setInputValueMatrix33f(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix33f>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueMatrix33f(const UniformInput& input, float valueOut[9]) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix33f>(input.impl, 1u, valueOut);
    }

    status_t Appearance::getInputValueMatrix33f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix33f>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputValueMatrix44f(const UniformInput& input, const float values[16])
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix44f>(input.impl, 1u, values);
    }

    status_t Appearance::setInputValueMatrix44f(const UniformInput& input, uint32_t elementCount, const float* values)
    {
        return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix44f>(input.impl, elementCount, values);
    }

    status_t Appearance::getInputValueMatrix44f(const UniformInput& input, float valueOut[16]) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix44f>(input.impl, 1u, valueOut);
    }

    status_t Appearance::getInputValueMatrix44f(const UniformInput& input, uint32_t elementCount, float* valuesOut) const
    {
        return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix44f>(input.impl, elementCount, valuesOut);
    }

    status_t Appearance::setInputTexture(const UniformInput& input, const TextureSampler& textureSampler)
    {
        const status_t status = impl.setInputTexture(input.impl, textureSampler.impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    status_t Appearance::getInputTexture(const UniformInput& input, const TextureSampler*& textureSampler) const
    {
        return impl.getInputTexture(input.impl, textureSampler);
    }

    status_t Appearance::bindInput(const UniformInput& input, const DataObject& dataObject)
    {
        const status_t status = impl.bindInput(input.impl, dataObject.impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(dataObject));
        return status;
    }

    status_t Appearance::unbindInput(const UniformInput& input)
    {
        const status_t status = impl.unbindInput(input.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_GENERIC_OBJECT_STRING(input));
        return status;
    }

    bool Appearance::isInputBound(const UniformInput& input) const
    {
        return impl.isInputBound(input.impl);
    }

    const Effect& Appearance::getEffect() const
    {
        return impl.getEffect();
    }
}
