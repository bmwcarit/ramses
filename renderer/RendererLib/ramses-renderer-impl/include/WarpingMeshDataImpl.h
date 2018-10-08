//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WARPINGMESHDATAIMPL_H
#define RAMSES_WARPINGMESHDATAIMPL_H

#include "StatusObjectImpl.h"
#include "RendererLib/WarpingMeshData.h"

namespace ramses
{
    class WarpingMeshDataImpl : public StatusObjectImpl
    {
    public:
        WarpingMeshDataImpl(uint32_t indexCount, const uint16_t* indices, uint32_t vertexCount, const float* vertexPositions, const float* textureCoordinates);

        const ramses_internal::WarpingMeshData& getWarpingMeshData() const;

    private:
        ramses_internal::WarpingMeshData m_warpingMeshData;
    };
}

#endif
