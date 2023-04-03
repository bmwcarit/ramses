//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/ArrayBuffer.h"

// Internal
#include "ArrayBufferImpl.h"

namespace ramses
{
    ArrayBuffer::ArrayBuffer(ArrayBufferImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    ArrayBuffer::~ArrayBuffer()
    {
    }

    template <typename T> status_t ArrayBuffer::updateDataInternal(uint32_t firstElement, uint32_t numElements, const T* bufferData)
    {
        if (GetEDataType<T>() != impl.getDataType())
            return impl.addErrorEntry("ArrayBuffer::updateData: Wrong data type used to update buffer!");

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) we store all data types as bytes internally
        const status_t status = impl.updateData(firstElement, numElements, reinterpret_cast<const ramses_internal::Byte*>(bufferData));
        LOG_HL_CLIENT_API3(status, firstElement, numElements, LOG_API_GENERIC_PTR_STRING(bufferData));
        return status;
    }

    uint32_t ArrayBuffer::getMaximumNumberOfElements() const
    {
        return impl.getMaximumNumberOfElements();
    }

    uint32_t ArrayBuffer::getUsedNumberOfElements() const
    {
        return impl.getUsedNumberOfElements();
    }

    EDataType ArrayBuffer::getDataType() const
    {
        return impl.getDataType();
    }

    template <typename T> status_t ArrayBuffer::getDataInternal(T* buffer, uint32_t numElements) const
    {
        if (GetEDataType<T>() != impl.getDataType())
            return impl.addErrorEntry("ArrayBuffer::getData: Wrong data type used to get data!");

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) we store all data types as bytes internally
        return impl.getData(reinterpret_cast<ramses_internal::Byte*>(buffer), numElements);
    }

    template RAMSES_API status_t ArrayBuffer::updateDataInternal<uint16_t>(uint32_t, uint32_t, const uint16_t*);
    template RAMSES_API status_t ArrayBuffer::updateDataInternal<uint32_t>(uint32_t, uint32_t, const uint32_t*);
    template RAMSES_API status_t ArrayBuffer::updateDataInternal<float>(uint32_t, uint32_t, const float*);
    template RAMSES_API status_t ArrayBuffer::updateDataInternal<vec2f>(uint32_t, uint32_t, const vec2f*);
    template RAMSES_API status_t ArrayBuffer::updateDataInternal<vec3f>(uint32_t, uint32_t, const vec3f*);
    template RAMSES_API status_t ArrayBuffer::updateDataInternal<vec4f>(uint32_t, uint32_t, const vec4f*);
    template RAMSES_API status_t ArrayBuffer::updateDataInternal<Byte>(uint32_t, uint32_t, const Byte*);

    template RAMSES_API status_t ArrayBuffer::getDataInternal<uint16_t>(uint16_t*, uint32_t) const;
    template RAMSES_API status_t ArrayBuffer::getDataInternal<uint32_t>(uint32_t*, uint32_t) const;
    template RAMSES_API status_t ArrayBuffer::getDataInternal<float>(float*, uint32_t) const;
    template RAMSES_API status_t ArrayBuffer::getDataInternal<vec2f>(vec2f*, uint32_t) const;
    template RAMSES_API status_t ArrayBuffer::getDataInternal<vec3f>(vec3f*, uint32_t) const;
    template RAMSES_API status_t ArrayBuffer::getDataInternal<vec4f>(vec4f*, uint32_t) const;
    template RAMSES_API status_t ArrayBuffer::getDataInternal<Byte>(Byte*, uint32_t) const;
}
