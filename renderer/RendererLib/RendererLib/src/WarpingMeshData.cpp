//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/WarpingMeshData.h"

namespace ramses_internal
{
    WarpingMeshData::WarpingMeshData()
    {
        m_vertexPositions.resize(4);
        m_textureCoordinates.resize(4);

        m_vertexPositions[0] = Vector3(-1.0f, -1.0f, 0.0f);
        m_vertexPositions[1] = Vector3( 1.0f, -1.0f, 0.0f);
        m_vertexPositions[2] = Vector3(-1.0f,  1.0f, 0.0f);
        m_vertexPositions[3] = Vector3( 1.0f,  1.0f, 0.0f);

        m_textureCoordinates[0] = Vector2(0.0f, 0.0f);
        m_textureCoordinates[1] = Vector2(1.0f, 0.0f);
        m_textureCoordinates[2] = Vector2(0.0f, 1.0f);
        m_textureCoordinates[3] = Vector2(1.0f, 1.0f);

        m_indices.resize(6);
        m_indices[0] = 0;
        m_indices[1] = 1;
        m_indices[2] = 2;
        m_indices[3] = 2;
        m_indices[4] = 1;
        m_indices[5] = 3;
    }

    WarpingMeshData::WarpingMeshData(UInt32 indexCount, const UInt16* indices, UInt32 vertexCount, const Float* vertexPositions, const Float* textureCoordinates)
    {
        m_indices.resize(indexCount);

        for (UInt i = 0; i < indexCount; ++i)
        {
            m_indices[i] = indices[i];
        }

        m_vertexPositions.resize(vertexCount);
        m_textureCoordinates.resize(vertexCount);

        for (UInt i = 0; i < vertexCount; ++i)
        {
            m_vertexPositions[i] = Vector3(vertexPositions[i * 3 + 0], vertexPositions[i * 3 + 1], vertexPositions[i * 3 + 2]);
            m_textureCoordinates[i] = Vector2(textureCoordinates[i * 2 + 0], textureCoordinates[i * 2 + 1]);
        }

    }

    const std::vector<UInt16>& WarpingMeshData::getIndices() const
    {
        return m_indices;
    }

    const std::vector<Vector3>& WarpingMeshData::getVertexPositions() const
    {
        return m_vertexPositions;
    }

    const std::vector<Vector2>& WarpingMeshData::getTextureCoordinates() const
    {
        return m_textureCoordinates;
    }


}
