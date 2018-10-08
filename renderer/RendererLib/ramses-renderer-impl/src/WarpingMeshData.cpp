//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-renderer-api/WarpingMeshData.h"

// internal
#include "WarpingMeshDataImpl.h"

namespace ramses
{
    WarpingMeshData::WarpingMeshData(uint32_t indexCount, const uint16_t* indices, uint32_t vertexCount, const float* vertexPositions, const float* textureCoordinates)
        : StatusObject(*new WarpingMeshDataImpl(indexCount, indices, vertexCount, vertexPositions, textureCoordinates))
        , impl(static_cast<WarpingMeshDataImpl&>(StatusObject::impl))
    {
    }

    WarpingMeshData::~WarpingMeshData()
    {
    }
}
