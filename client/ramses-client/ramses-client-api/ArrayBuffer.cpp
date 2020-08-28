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

    status_t ArrayBuffer::updateData(uint32_t firstElement, uint32_t numElements, const void* bufferData)
    {
        const status_t status = impl.updateData(firstElement, numElements, reinterpret_cast<const ramses_internal::Byte*>(bufferData));
        LOG_HL_CLIENT_API3(status, firstElement, numElements, bufferData);
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

    status_t ArrayBuffer::getData(void* buffer, uint32_t numElements) const
    {
        return impl.getData(reinterpret_cast<ramses_internal::Byte*>(buffer), numElements);
    }
}
