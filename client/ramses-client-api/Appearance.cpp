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

namespace ramses
{
    Appearance::Appearance(std::unique_ptr<AppearanceImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<AppearanceImpl&>(SceneObject::m_impl) }
    {
    }

    status_t Appearance::setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        const status_t status = m_impl.setBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        LOG_HL_CLIENT_API4(status, srcColor, destColor, srcAlpha, destAlpha);
        return status;
    }

    status_t Appearance::getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const
    {
        return m_impl.getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
    }

    status_t Appearance::setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        const status_t status = m_impl.setBlendingOperations(operationColor, operationAlpha);
        LOG_HL_CLIENT_API2(status, operationColor, operationAlpha);
        return status;
    }

    status_t Appearance::getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const
    {
        return m_impl.getBlendingOperations(operationColor, operationAlpha);
    }

    ramses::status_t Appearance::setBlendingColor(const vec4f& color)
    {
        const status_t status = m_impl.setBlendingColor(color);
        LOG_HL_CLIENT_API4(status, color[0], color[1], color[2], color[4]);
        return status;
    }

    ramses::status_t Appearance::getBlendingColor(vec4f& color) const
    {
        return m_impl.getBlendingColor(color);
    }

    status_t Appearance::setDepthWrite(EDepthWrite mode)
    {
        const status_t status = m_impl.setDepthWrite(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    status_t Appearance::getDepthWriteMode(EDepthWrite& mode) const
    {
        return m_impl.getDepthWriteMode(mode);
    }

    status_t Appearance::setDepthFunction(EDepthFunc func)
    {
        const status_t status = m_impl.setDepthFunction(func);
        LOG_HL_CLIENT_API1(status, func);
        return status;
    }

    status_t Appearance::getDepthFunction(EDepthFunc& func) const
    {
        return m_impl.getDepthFunction(func);
    }

    status_t Appearance::setScissorTest(EScissorTest state, int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        const status_t status = m_impl.setScissorTest(state, x, y, width, height);
        LOG_HL_CLIENT_API5(status, state, x, y, width, height);
        return status;
    }

    status_t Appearance::getScissorTestState(EScissorTest& state) const
    {
        return m_impl.getScissorTestState(state);
    }

    status_t Appearance::getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const
    {
        return m_impl.getScissorRegion(x, y, width, height);
    }

    status_t Appearance::setStencilFunction(EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        const status_t status = m_impl.setStencilFunc(func, ref, mask);
        LOG_HL_CLIENT_API3(status, func, ref, mask);
        return status;
    }

    status_t Appearance::getStencilFunction(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const
    {
        return m_impl.getStencilFunc(func, ref, mask);
    }

    status_t Appearance::setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass)
    {
        const status_t status = m_impl.setStencilOperation(sfail, dpfail, dppass);
        LOG_HL_CLIENT_API3(status, sfail, dpfail, dppass);
        return status;
    }

    status_t Appearance::getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const
    {
        return m_impl.getStencilOperation(sfail, dpfail, dppass);
    }

    status_t Appearance::setCullingMode(ECullMode mode)
    {
        const status_t status = m_impl.setCullingMode(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    status_t Appearance::getCullingMode(ECullMode& mode) const
    {
        return m_impl.getCullingMode(mode);
    }

    status_t Appearance::setDrawMode(EDrawMode mode)
    {
        const status_t status = m_impl.setDrawMode(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    status_t Appearance::getDrawMode(EDrawMode& mode) const
    {
        return m_impl.getDrawMode(mode);
    }

    status_t Appearance::setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
    {
        const status_t status = m_impl.setColorWriteMask(writeRed, writeGreen, writeBlue, writeAlpha);
        LOG_HL_CLIENT_API4(status, writeRed, writeGreen, writeBlue, writeAlpha);
        return status;
    }

    status_t Appearance::getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const
    {
        return m_impl.getColorWriteMask(writeRed, writeGreen, writeBlue, writeAlpha);
    }

    status_t Appearance::setInputTexture(const UniformInput& input, const TextureSampler& textureSampler)
    {
        const status_t status = m_impl.setInputTexture(input.m_impl, textureSampler.m_impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    status_t Appearance::getInputTexture(const UniformInput& input, const TextureSampler*& textureSampler) const
    {
        return m_impl.getInputTexture(input.m_impl, textureSampler);
    }

    status_t Appearance::setInputTexture(const UniformInput& input, const TextureSamplerMS& textureSampler)
    {
        const status_t status = m_impl.setInputTexture(input.m_impl, textureSampler.m_impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    status_t Appearance::getInputTextureMS(const UniformInput& input, const TextureSamplerMS*& textureSampler) const
    {
        return m_impl.getInputTextureMS(input.m_impl, textureSampler);
    }

    status_t Appearance::setInputTexture(const UniformInput& input, const TextureSamplerExternal& textureSampler)
    {
        const status_t status = m_impl.setInputTexture(input.m_impl, textureSampler.m_impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    status_t Appearance::getInputTextureExternal(const UniformInput& input, const TextureSamplerExternal*& textureSampler) const
    {
        return m_impl.getInputTextureExternal(input.m_impl, textureSampler);
    }

    status_t Appearance::bindInput(const UniformInput& input, const DataObject& dataObject)
    {
        const status_t status = m_impl.bindInput(input.m_impl, dataObject.m_impl);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(dataObject));
        return status;
    }

    status_t Appearance::unbindInput(const UniformInput& input)
    {
        const status_t status = m_impl.unbindInput(input.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_GENERIC_OBJECT_STRING(input));
        return status;
    }

    bool Appearance::isInputBound(const UniformInput& input) const
    {
        return m_impl.isInputBound(input.m_impl);
    }

    const DataObject* Appearance::getDataObjectBoundToInput(const UniformInput& input) const
    {
        return m_impl.getBoundDataObject(input.m_impl);
    }

    const Effect& Appearance::getEffect() const
    {
        return m_impl.getEffect();
    }

    template <typename T> status_t Appearance::setInputValueInternal(const UniformInput& input, T&& value)
    {
        // API uses ref/move forwarding for possibility to move but current implementation does not make use of it
        return setInputValueInternal(input, 1u, &value);
    }

    template <typename T> status_t Appearance::setInputValueInternal(const UniformInput& input, size_t elementCount, const T* values)
    {
        return m_impl.setInputValue(input.m_impl, elementCount, values);
    }

    template <typename T> status_t Appearance::getInputValueInternal(const UniformInput& input, T& value) const
    {
        return getInputValueInternal(input, 1u, &value);
    }

    template <typename T> status_t Appearance::getInputValueInternal(const UniformInput& input, size_t elementCount, T* valuesOut) const
    {
        return m_impl.getInputValue(input.m_impl, elementCount, valuesOut);
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

    template RAMSES_API status_t Appearance::setInputValueInternal<int32_t>(const UniformInput&, size_t, const int32_t*);
    template RAMSES_API status_t Appearance::setInputValueInternal<float>(const UniformInput&, size_t, const float*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2i>(const UniformInput&, size_t, const vec2i*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3i>(const UniformInput&, size_t, const vec3i*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4i>(const UniformInput&, size_t, const vec4i*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec2f>(const UniformInput&, size_t, const vec2f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec3f>(const UniformInput&, size_t, const vec3f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<vec4f>(const UniformInput&, size_t, const vec4f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix22f>(const UniformInput&, size_t, const matrix22f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix33f>(const UniformInput&, size_t, const matrix33f*);
    template RAMSES_API status_t Appearance::setInputValueInternal<matrix44f>(const UniformInput&, size_t, const matrix44f*);

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

    template RAMSES_API status_t Appearance::getInputValueInternal<int32_t>(const UniformInput&, size_t, int32_t*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<float>(const UniformInput&, size_t, float*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec2i>(const UniformInput&, size_t, vec2i*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec3i>(const UniformInput&, size_t, vec3i*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec4i>(const UniformInput&, size_t, vec4i*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec2f>(const UniformInput&, size_t, vec2f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec3f>(const UniformInput&, size_t, vec3f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<vec4f>(const UniformInput&, size_t, vec4f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix22f>(const UniformInput&, size_t, matrix22f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix33f>(const UniformInput&, size_t, matrix33f*) const;
    template RAMSES_API status_t Appearance::getInputValueInternal<matrix44f>(const UniformInput&, size_t, matrix44f*) const;
}
