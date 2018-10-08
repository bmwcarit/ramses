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

        const Vector<UInt16>& getIndices() const;
        const Vector<Vector3>& getVertexPositions() const;
        const Vector<Vector2>& getTextureCoordinates() const;

    private:
        Vector<UInt16> m_indices;
        Vector<Vector3> m_vertexPositions;
        Vector<Vector2> m_textureCoordinates;
    };
}

#endif
