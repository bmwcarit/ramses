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

    template <typename T> bool ArrayBuffer::updateDataInternal(size_t firstElement, size_t numElements, const T* bufferData)
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

    size_t ArrayBuffer::getMaximumNumberOfElements() const
    {
        return m_impl.getMaximumNumberOfElements();
    }

    size_t ArrayBuffer::getUsedNumberOfElements() const
    {
        return m_impl.getUsedNumberOfElements();
    }

    EDataType ArrayBuffer::getDataType() const
    {
        return m_impl.getDataType();
    }

    template <typename T> bool ArrayBuffer::getDataInternal(T* buffer, size_t numElements) const
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

    template RAMSES_API bool ArrayBuffer::updateDataInternal<uint16_t>(size_t, size_t, const uint16_t*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<uint32_t>(size_t, size_t, const uint32_t*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<float>(size_t, size_t, const float*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<vec2f>(size_t, size_t, const vec2f*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<vec3f>(size_t, size_t, const vec3f*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<vec4f>(size_t, size_t, const vec4f*);
    template RAMSES_API bool ArrayBuffer::updateDataInternal<std::byte>(size_t, size_t, const std::byte*);

    template RAMSES_API bool ArrayBuffer::getDataInternal<uint16_t>(uint16_t*, size_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<uint32_t>(uint32_t*, size_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<float>(float*, size_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<vec2f>(vec2f*, size_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<vec3f>(vec3f*, size_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<vec4f>(vec4f*, size_t) const;
    template RAMSES_API bool ArrayBuffer::getDataInternal<std::byte>(std::byte*, size_t) const;
}
