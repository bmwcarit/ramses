//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WARPINGMESHDATA_H
#define RAMSES_WARPINGMESHDATA_H

#include "RendererAPI/Types.h"

#include "Math3d/Vector3.h"
#include "Math3d/Vector2.h"

namespace ramses_internal
{
    class WarpingMeshData
    {
    public:
        WarpingMeshData();
        WarpingMeshData(UInt32 indexCount, const UInt16* indices, UInt32 vertexCount, const Float* vertexPositions, const Float* textureCoordinates);

        const std::vector<UInt16>& getIndices() const;
        const std::vector<Vector3>& getVertexPositions() const;
        const std::vector<Vector2>& getTextureCoordinates() const;

    private:
        std::vector<UInt16> m_indices;
        std::vector<Vector3> m_vertexPositions;
        std::vector<Vector2> m_textureCoordinates;
    };
}

#endif
