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
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
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

    ramses::status_t Appearance::setBlendingColor(const vec4f& color)
    {
        const status_t status = impl.setBlendingColor(color);
        LOG_HL_CLIENT_API4(status, color[0], color[1], color[2], color[4]);
        return status;
    }

    ramses::status_t Appearance::getBlendingColor(vec4f& color) const
    {
        return impl.getBlendingColor(color);
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

    status_t Appearance::setScissorTest(EScissorTest state, int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        const status_t status = impl.setScissorTest(state, x, y, width, height);
        LOG_HL_CLIENT_API5(status, state, x, y, width, height);
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

    status_t Appearance::setInputTexture(const UniformInput& input, const TextureSamplerMS& textureSampler)
    {
        const status_t status = impl.setInputTexture(input.impl, textureSampler.impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    status_t Appearance::getInputTextureMS(const UniformInput& input, const TextureSamplerMS*& textureSampler) const
    {
        return impl.getInputTextureMS(input.impl, textureSampler);
    }

    status_t Appearance::setInputTexture(const UniformInput& input, const TextureSamplerExternal& textureSampler)
    {
        const status_t status = impl.setInputTexture(input.impl, textureSampler.impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    status_t Appearance::getInputTextureExternal(const UniformInput& input, const TextureSamplerExternal*& textureSampler) const
    {
        return impl.getInputTextureExternal(input.impl, textureSampler);
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

    const DataObject* Appearance::getDataObjectBoundToInput(const UniformInput& input) const
    {
        return impl.getBoundDataObject(input.impl);
    }

    const Effect& Appearance::getEffect() const
    {
        return impl.getEffect();
    }

    template <typename T> status_t Appearance::setInputValueInternal(const UniformInput& input, T&& value)
    {
        // API uses ref/move forwarding for possibility to move but current implementation does not make use of it
        return setInputValueInternal(input, 1u, &value);
    }

    template <typename T> status_t Appearance::setInputValueInternal(const UniformInput& input, uint32_t elementCount, const T* values)
    {
        if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, float>)
            return impl.setInputValue(input.impl, elementCount, values);

        if constexpr (std::is_same_v<T, vec2i>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Vector2i>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, vec3i>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Vector3i>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, vec4i>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Vector4i>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, vec2f>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Vector2>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, vec3f>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Vector3>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, vec4f>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Vector4>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, matrix22f>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix22f>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, matrix33f>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix33f>(input.impl, elementCount, values);
        if constexpr (std::is_same_v<T, matrix44f>)
            return impl.setInputValueWithElementTypeCast<ramses_internal::Matrix44f>(input.impl, elementCount, values);
    }

    template <typename T> status_t Appearance::getInputValueInternal(const UniformInput& input, T& value) const
    {
        return getInputValueInternal(input, 1u, &value);
    }

    template <typename T> status_t Appearance::getInputValueInternal(const UniformInput& input, uint32_t elementCount, T* valuesOut) const
    {
        if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, float>)
            return impl.getInputValue(input.impl, elementCount, valuesOut);

        if constexpr (std::is_same_v<T, vec2i>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Vector2i>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, vec3i>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Vector3i>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, vec4i>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Vector4i>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, vec2f>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Vector2>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, vec3f>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Vector3>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, vec4f>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Vector4>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, matrix22f>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix22f>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, matrix33f>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix33f>(input.impl, elementCount, valuesOut);
        if constexpr (std::is_same_v<T, matrix44f>)
            return impl.getInputValueWithElementTypeCast<ramses_internal::Matrix44f>(input.impl, elementCount, valuesOut);
    }

    // const l-value instances
    template RAMSES_API status_t Appearance::setInputValueInternal<const int32_t&>(const UniformInput&, const int32_t&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const float&>(const UniformInput&, const float&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const vec2i&>(const UniformInput&, const vec2i&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const vec3i&>(const UniformInput&, const vec3i&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const vec4i&>(const UniformInput&, const vec4i&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const vec2f&>(const UniformInput&, const vec2f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const vec3f&>(const UniformInput&, const vec3f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const vec4f&>(const UniformInput&, const vec4f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const matrix22f&>(const UniformInput&, const matrix22f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const matrix33f&>(const UniformInput&, const matrix33f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<const matrix44f&>(const UniformInput&, const matrix44f&);

    // l-value instances
    template RAMSES_API status_t Appearance::setInputValueInternal<int32_t&>(const UniformInput&, int32_t&);
    template RAMSES_API status_t Appearance::setInputValueInternal<float&>(const UniformInput&, float&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2i&>(const UniformInput&, vec2i&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3i&>(const UniformInput&, vec3i&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4i&>(const UniformInput&, vec4i&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2f&>(const UniformInput&, vec2f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3f&>(const UniformInput&, vec3f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4f&>(const UniformInput&, vec4f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix22f&>(const UniformInput&, matrix22f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix33f&>(const UniformInput&, matrix33f&);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix44f&>(const UniformInput&, matrix44f&);

    // r-value instances
    template RAMSES_API status_t Appearance::setInputValueInternal<int32_t>(const UniformInput&, int32_t&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<float>(const UniformInput&, float&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2i>(const UniformInput&, vec2i&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3i>(const UniformInput&, vec3i&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4i>(const UniformInput&, vec4i&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2f>(const UniformInput&, vec2f&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3f>(const UniformInput&, vec3f&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4f>(const UniformInput&, vec4f&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix22f>(const UniformInput&, matrix22f&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix33f>(const UniformInput&, matrix33f&&);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix44f>(const UniformInput&, matrix44f&&);

    template RAMSES_API status_t Appearance::setInputValueInternal<int32_t>(const UniformInput&, uint32_t, const int32_t*);
    template RAMSES_API status_t Appearance::setInputValueInternal<float>(const UniformInput&, uint32_t, const float*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2i>(const UniformInput&, uint32_t, const vec2i*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3i>(const UniformInput&, uint32_t, const vec3i*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4i>(const UniformInput&, uint32_t, const vec4i*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2f>(const UniformInput&, uint32_t, const vec2f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3f>(const UniformInput&, uint32_t, const vec3f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4f>(const UniformInput&, uint32_t, const vec4f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix22f>(const UniformInput&, uint32_t, const matrix22f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix33f>(const UniformInput&, uint32_t, const matrix33f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix44f>(const UniformInput&, uint32_t, const matrix44f*);

    template RAMSES_API status_t Appearance::getInputValueInternal<int32_t>(const UniformInput&, int32_t&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<float>(const UniformInput&, float&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec2i>(const UniformInput&, vec2i&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec3i>(const UniformInput&, vec3i&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec4i>(const UniformInput&, vec4i&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec2f>(const UniformInput&, vec2f&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec3f>(const UniformInput&, vec3f&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec4f>(const UniformInput&, vec4f&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix22f>(const UniformInput&, matrix22f&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix33f>(const UniformInput&, matrix33f&) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix44f>(const UniformInput&, matrix44f&) const;

    template RAMSES_API status_t Appearance::getInputValueInternal<int32_t>(const UniformInput&, uint32_t, int32_t*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<float>(const UniformInput&, uint32_t, float*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec2i>(const UniformInput&, uint32_t, vec2i*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec3i>(const UniformInput&, uint32_t, vec3i*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec4i>(const UniformInput&, uint32_t, vec4i*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec2f>(const UniformInput&, uint32_t, vec2f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec3f>(const UniformInput&, uint32_t, vec3f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec4f>(const UniformInput&, uint32_t, vec4f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix22f>(const UniformInput&, uint32_t, matrix22f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix33f>(const UniformInput&, uint32_t, matrix33f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix44f>(const UniformInput&, uint32_t, matrix44f*) const;
}
