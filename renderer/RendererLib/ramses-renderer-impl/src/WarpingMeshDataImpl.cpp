//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WarpingMeshDataImpl.h"

namespace ramses
{
    WarpingMeshDataImpl::WarpingMeshDataImpl(uint32_t indexCount, const uint16_t* indices, uint32_t vertexCount, const float* vertexPositions, const float* textureCoordinates)
        : StatusObjectImpl()
        , m_warpingMeshData(indexCount, indices, vertexCount, vertexPositions, textureCoordinates)
    {
    }

    const ramses_internal::WarpingMeshData& WarpingMeshDataImpl::getWarpingMeshData() const
    {
        return m_warpingMeshData;
    }
}
