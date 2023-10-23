//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/ArrayBuffer.h"

// Internal
#include "impl/ArrayBufferImpl.h"
#include "impl/ErrorReporting.h"

namespace ramses
{
    ArrayBuffer::ArrayBuffer(std::unique_ptr<internal::ArrayBufferImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::ArrayBufferImpl&>(SceneObject::m_impl) }
    {
    }

    template <typename T> bool ArrayBuffer::updateDataInternal(uint32_t firstElement, uint32_t numElements, const T* bufferData)
    {
        if (GetEDataType<T>() != m_impl.getDataType())
        {
            m_impl.getErrorReporting().set("ArrayBuffer::updateData: Wrong data type used to update buffer!");
            return false;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) we store all data types as bytes internally
        const bool status = m_impl.updateData(firstElement, numElements, reinterpret_cast<const std::byte*>(bufferData));
        LOG_HL_CLIENT_API3(status, firstElement, numElements, LOG_API_GENERIC_PTR_STRING(bufferData));
        return status;
    }

    uint32_t ArrayBuffer::getMaximumNumberOfElements() const
    {
        return m_impl.getMaximumNumberOfElements();
    }

    uint32_t ArrayBuffer::getUsedNumberOfElements() const
    {
        return m_impl.getUsedNumberOfElements();
    }

    EDataType ArrayBuffer::getDataType() const
    {
        return m_impl.getDataType();
    }

    template <typename T> bool ArrayBuffer::getDataInternal(T* buffer, uint32_t numElements) const
    {
        if (GetEDataType<T>() != m_impl.getDataType())
        {
            m_impl.getErrorReporting().set("ArrayBuffer::getData: Wrong data type used to get data!");
            return false;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) we store all data types as bytes internally
        return m_impl.getData(reinterpret_cast<std::byte*>(buffer), numElements);
    }

    internal::ArrayBufferImpl& ArrayBuffer::impl()
    {
        return m_impl;
    }

    const internal::ArrayBufferImpl& ArrayBuffer::impl() const
    {
        return m_impl;
    }

    template RAMSES_API bool ArrayBuffer::updateDataInternal<uint16_t>(uint32_t, uint32_t, const uint16_t*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<uint32_t>(uint32_t, uint32_t, const uint32_t*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<float>(uint32_t, uint32_t, const float*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<vec2f>(uint32_t, uint32_t, const vec2f*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<vec3f>(uint32_t, uint32_t, const vec3f*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<vec4f>(uint32_t, uint32_t, const vec4f*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<std::byte>(uint32_t, uint32_t, const std::byte*);

    template RAMSES_API bool ArrayBuffer::getDataInternal<uint16_t>(uint16_t*, uint32_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<uint32_t>(uint32_t*, uint32_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<float>(float*, uint32_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<vec2f>(vec2f*, uint32_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<vec3f>(vec3f*, uint32_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<vec4f>(vec4f*, uint32_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<std::byte>(std::byte*, uint32_t) const;
}
