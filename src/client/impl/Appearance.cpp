//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Appearance.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/DataObject.h"

// internal
#include "impl/AppearanceImpl.h"

namespace ramses
{
    Appearance::Appearance(std::unique_ptr<internal::AppearanceImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::AppearanceImpl&>(SceneObject::m_impl) }
    {
    }

    bool Appearance::setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        const bool status = m_impl.setBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        LOG_HL_CLIENT_API4(status, srcColor, destColor, srcAlpha, destAlpha);
        return status;
    }

    bool Appearance::getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const
    {
        return m_impl.getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
    }

    bool Appearance::setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        const bool status = m_impl.setBlendingOperations(operationColor, operationAlpha);
        LOG_HL_CLIENT_API2(status, operationColor, operationAlpha);
        return status;
    }

    bool Appearance::getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const
    {
        return m_impl.getBlendingOperations(operationColor, operationAlpha);
    }

    bool Appearance::setBlendingColor(const vec4f& color)
    {
        const bool status = m_impl.setBlendingColor(color);
        LOG_HL_CLIENT_API4(status, color[0], color[1], color[2], color[3]);
        return status;
    }

    bool Appearance::getBlendingColor(vec4f& color) const
    {
        return m_impl.getBlendingColor(color);
    }

    bool Appearance::setDepthWrite(EDepthWrite mode)
    {
        const bool status = m_impl.setDepthWrite(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    bool Appearance::getDepthWriteMode(EDepthWrite& mode) const
    {
        return m_impl.getDepthWriteMode(mode);
    }

    bool Appearance::setDepthFunction(EDepthFunc func)
    {
        const bool status = m_impl.setDepthFunction(func);
        LOG_HL_CLIENT_API1(status, func);
        return status;
    }

    bool Appearance::getDepthFunction(EDepthFunc& func) const
    {
        return m_impl.getDepthFunction(func);
    }

    bool Appearance::setScissorTest(EScissorTest state, int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        const bool status = m_impl.setScissorTest(state, x, y, width, height);
        LOG_HL_CLIENT_API5(status, state, x, y, width, height);
        return status;
    }

    bool Appearance::getScissorTestState(EScissorTest& state) const
    {
        return m_impl.getScissorTestState(state);
    }

    bool Appearance::getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const
    {
        return m_impl.getScissorRegion(x, y, width, height);
    }

    bool Appearance::setStencilFunction(EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        const bool status = m_impl.setStencilFunc(func, ref, mask);
        LOG_HL_CLIENT_API3(status, func, ref, mask);
        return status;
    }

    bool Appearance::getStencilFunction(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const
    {
        return m_impl.getStencilFunc(func, ref, mask);
    }

    bool Appearance::setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass)
    {
        const bool status = m_impl.setStencilOperation(sfail, dpfail, dppass);
        LOG_HL_CLIENT_API3(status, sfail, dpfail, dppass);
        return status;
    }

    bool Appearance::getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const
    {
        return m_impl.getStencilOperation(sfail, dpfail, dppass);
    }

    bool Appearance::setCullingMode(ECullMode mode)
    {
        const bool status = m_impl.setCullingMode(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    bool Appearance::getCullingMode(ECullMode& mode) const
    {
        return m_impl.getCullingMode(mode);
    }

    bool Appearance::setDrawMode(EDrawMode mode)
    {
        const bool status = m_impl.setDrawMode(mode);
        LOG_HL_CLIENT_API1(status, mode);
        return status;
    }

    bool Appearance::getDrawMode(EDrawMode& mode) const
    {
        return m_impl.getDrawMode(mode);
    }

    bool Appearance::setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
    {
        const bool status = m_impl.setColorWriteMask(writeRed, writeGreen, writeBlue, writeAlpha);
        LOG_HL_CLIENT_API4(status, writeRed, writeGreen, writeBlue, writeAlpha);
        return status;
    }

    bool Appearance::getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const
    {
        return m_impl.getColorWriteMask(writeRed, writeGreen, writeBlue, writeAlpha);
    }

    bool Appearance::setInputTexture(const UniformInput& input, const TextureSampler& textureSampler)
    {
        const bool status = m_impl.setInputTexture(input.impl(), textureSampler.impl());
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    bool Appearance::getInputTexture(const UniformInput& input, const TextureSampler*& textureSampler) const
    {
        return m_impl.getInputTexture(input.impl(), textureSampler);
    }

    bool Appearance::setInputTexture(const UniformInput& input, const TextureSamplerMS& textureSampler)
    {
        const bool status = m_impl.setInputTexture(input.impl(), textureSampler.impl());
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    bool Appearance::getInputTextureMS(const UniformInput& input, const TextureSamplerMS*& textureSampler) const
    {
        return m_impl.getInputTextureMS(input.impl(), textureSampler);
    }

    bool Appearance::setInputTexture(const UniformInput& input, const TextureSamplerExternal& textureSampler)
    {
        const bool status = m_impl.setInputTexture(input.impl(), textureSampler.impl());
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(textureSampler));
        return status;
    }

    bool Appearance::getInputTextureExternal(const UniformInput& input, const TextureSamplerExternal*& textureSampler) const
    {
        return m_impl.getInputTextureExternal(input.impl(), textureSampler);
    }

    bool Appearance::bindInput(const UniformInput& input, const DataObject& dataObject)
    {
        const bool status = m_impl.bindInput(input.impl(), dataObject.impl());
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_OBJECT_STRING(input), LOG_API_RAMSESOBJECT_STRING(dataObject));
        return status;
    }

    bool Appearance::unbindInput(const UniformInput& input)
    {
        const bool status = m_impl.unbindInput(input.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_GENERIC_OBJECT_STRING(input));
        return status;
    }

    bool Appearance::isInputBound(const UniformInput& input) const
    {
        return m_impl.isInputBound(input.impl());
    }

    const DataObject* Appearance::getDataObjectBoundToInput(const UniformInput& input) const
    {
        return m_impl.getBoundDataObject(input.impl());
    }

    const Effect& Appearance::getEffect() const
    {
        return m_impl.getEffect();
    }

    internal::AppearanceImpl& Appearance::impl()
    {
        return m_impl;
    }

    const internal::AppearanceImpl& Appearance::impl() const
    {
        return m_impl;
    }

    template <typename T> bool Appearance::setInputValueInternal(const UniformInput& input, T&& value)
    {
        // API uses ref/move forwarding for possibility to move but current implementation does not make use of it
        return setInputValueInternal(input, 1u, &value);
    }

    template <typename T> bool Appearance::setInputValueInternal(const UniformInput& input, size_t elementCount, const T* values)
    {
        return m_impl.setInputValue(input.impl(), elementCount, values);
    }

    template <typename T> bool Appearance::getInputValueInternal(const UniformInput& input, T& value) const
    {
        return getInputValueInternal(input, 1u, &value);
    }

    template <typename T> bool Appearance::getInputValueInternal(const UniformInput& input, size_t elementCount, T* valuesOut) const
    {
        return m_impl.getInputValue(input.impl(), elementCount, valuesOut);
    }

    // const l-value instances
    template RAMSES_API bool Appearance::setInputValueInternal<const bool&>(const UniformInput&, const bool&);
    template RAMSES_API bool Appearance::setInputValueInternal<const int32_t&>(const UniformInput&, const int32_t&);
    template RAMSES_API bool Appearance::setInputValueInternal<const float&>(const UniformInput&, const float&);
    template RAMSES_API bool Appearance::setInputValueInternal<const vec2i&>(const UniformInput&, const vec2i&);
    template RAMSES_API bool Appearance::setInputValueInternal<const vec3i&>(const UniformInput&, const vec3i&);
    template RAMSES_API bool Appearance::setInputValueInternal<const vec4i&>(const UniformInput&, const vec4i&);
    template RAMSES_API bool Appearance::setInputValueInternal<const vec2f&>(const UniformInput&, const vec2f&);
    template RAMSES_API bool Appearance::setInputValueInternal<const vec3f&>(const UniformInput&, const vec3f&);
    template RAMSES_API bool Appearance::setInputValueInternal<const vec4f&>(const UniformInput&, const vec4f&);
    template RAMSES_API bool Appearance::setInputValueInternal<const matrix22f&>(const UniformInput&, const matrix22f&);
    template RAMSES_API bool Appearance::setInputValueInternal<const matrix33f&>(const UniformInput&, const matrix33f&);
    template RAMSES_API bool Appearance::setInputValueInternal<const matrix44f&>(const UniformInput&, const matrix44f&);

    // l-value instances
    template RAMSES_API bool Appearance::setInputValueInternal<bool&>(const UniformInput&, bool&);
    template RAMSES_API bool Appearance::setInputValueInternal<int32_t&>(const UniformInput&, int32_t&);
    template RAMSES_API bool Appearance::setInputValueInternal<float&>(const UniformInput&, float&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec2i&>(const UniformInput&, vec2i&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec3i&>(const UniformInput&, vec3i&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec4i&>(const UniformInput&, vec4i&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec2f&>(const UniformInput&, vec2f&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec3f&>(const UniformInput&, vec3f&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec4f&>(const UniformInput&, vec4f&);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix22f&>(const UniformInput&, matrix22f&);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix33f&>(const UniformInput&, matrix33f&);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix44f&>(const UniformInput&, matrix44f&);

    // r-value instances
    template RAMSES_API bool Appearance::setInputValueInternal<bool>(const UniformInput&, bool&&);
    template RAMSES_API bool Appearance::setInputValueInternal<int32_t>(const UniformInput&, int32_t&&);
    template RAMSES_API bool Appearance::setInputValueInternal<float>(const UniformInput&, float&&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec2i>(const UniformInput&, vec2i&&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec3i>(const UniformInput&, vec3i&&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec4i>(const UniformInput&, vec4i&&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec2f>(const UniformInput&, vec2f&&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec3f>(const UniformInput&, vec3f&&);
    template RAMSES_API bool Appearance::setInputValueInternal<vec4f>(const UniformInput&, vec4f&&);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix22f>(const UniformInput&, matrix22f&&);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix33f>(const UniformInput&, matrix33f&&);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix44f>(const UniformInput&, matrix44f&&);

    template RAMSES_API bool Appearance::setInputValueInternal<bool>(const UniformInput&, size_t, const bool*);
    template RAMSES_API bool Appearance::setInputValueInternal<int32_t>(const UniformInput&, size_t, const int32_t*);
    template RAMSES_API bool Appearance::setInputValueInternal<float>(const UniformInput&, size_t, const float*);
    template RAMSES_API bool Appearance::setInputValueInternal<vec2i>(const UniformInput&, size_t, const vec2i*);
    template RAMSES_API bool Appearance::setInputValueInternal<vec3i>(const UniformInput&, size_t, const vec3i*);
    template RAMSES_API bool Appearance::setInputValueInternal<vec4i>(const UniformInput&, size_t, const vec4i*);
    template RAMSES_API bool Appearance::setInputValueInternal<vec2f>(const UniformInput&, size_t, const vec2f*);
    template RAMSES_API bool Appearance::setInputValueInternal<vec3f>(const UniformInput&, size_t, const vec3f*);
    template RAMSES_API bool Appearance::setInputValueInternal<vec4f>(const UniformInput&, size_t, const vec4f*);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix22f>(const UniformInput&, size_t, const matrix22f*);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix33f>(const UniformInput&, size_t, const matrix33f*);
    template RAMSES_API bool Appearance::setInputValueInternal<matrix44f>(const UniformInput&, size_t, const matrix44f*);

    template RAMSES_API bool Appearance::getInputValueInternal<bool>(const UniformInput&, bool&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<int32_t>(const UniformInput&, int32_t&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<float>(const UniformInput&, float&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec2i>(const UniformInput&, vec2i&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec3i>(const UniformInput&, vec3i&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec4i>(const UniformInput&, vec4i&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec2f>(const UniformInput&, vec2f&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec3f>(const UniformInput&, vec3f&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec4f>(const UniformInput&, vec4f&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<matrix22f>(const UniformInput&, matrix22f&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<matrix33f>(const UniformInput&, matrix33f&) const;
    template RAMSES_API bool Appearance::getInputValueInternal<matrix44f>(const UniformInput&, matrix44f&) const;

    template RAMSES_API bool Appearance::getInputValueInternal<bool>(const UniformInput&, size_t, bool*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<int32_t>(const UniformInput&, size_t, int32_t*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<float>(const UniformInput&, size_t, float*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec2i>(const UniformInput&, size_t, vec2i*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec3i>(const UniformInput&, size_t, vec3i*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec4i>(const UniformInput&, size_t, vec4i*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec2f>(const UniformInput&, size_t, vec2f*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec3f>(const UniformInput&, size_t, vec3f*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<vec4f>(const UniformInput&, size_t, vec4f*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<matrix22f>(const UniformInput&, size_t, matrix22f*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<matrix33f>(const UniformInput&, size_t, matrix33f*) const;
    template RAMSES_API bool Appearance::getInputValueInternal<matrix44f>(const UniformInput&, size_t, matrix44f*) const;
}
