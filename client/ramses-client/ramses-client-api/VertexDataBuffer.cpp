//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/VertexDataBuffer.h"

// Internal
#include "VertexDataBufferImpl.h"

namespace ramses
{
    VertexDataBuffer::VertexDataBuffer(VertexDataBufferImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    VertexDataBuffer::~VertexDataBuffer()
    {
    }

    status_t VertexDataBuffer::setData(const char* data, uint32_t dataSizeInBytes, uint32_t offsetInBytes)
    {
        const status_t status = impl.setData(reinterpret_cast<const ramses_internal::Byte*>(data), dataSizeInBytes, offsetInBytes);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_PTR_STRING(data), dataSizeInBytes, offsetInBytes);
        return status;
    }

    uint32_t VertexDataBuffer::getMaximumSizeInBytes() const
    {
        return impl.getMaximumSizeInBytes();
    }

    uint32_t VertexDataBuffer::getUsedSizeInBytes() const
    {
        return impl.getUsedSizeInBytes();
    }

    EDataType VertexDataBuffer::getDataType() const
    {
        return impl.getDataType();
    }

    status_t VertexDataBuffer::getData(char* buffer, uint32_t bufferSize) const
    {
        return impl.getData(reinterpret_cast<ramses_internal::Byte*>(buffer), bufferSize);
    }
}
