//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATABUFFERSCENE_H
#define RAMSES_DATABUFFERSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"
#include "Collections/Vector.h"

namespace ramses
{
    class Effect;
    class IndexDataBuffer;
    class VertexDataBuffer;
}

namespace ramses_internal
{
    class Vector4;

    class DataBufferScene : public IntegrationScene
    {
    public:
        DataBufferScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            INDEX_DATA_BUFFER_UINT16 = 0,
            INDEX_DATA_BUFFER_UINT32,

            VERTEX_DATA_BUFFER_FLOAT,
            VERTEX_DATA_BUFFER_VECTOR2F,
            VERTEX_DATA_BUFFER_VECTOR3F,
            VERTEX_DATA_BUFFER_VECTOR4F,
            VERTEX_ARRAY_BUFFER_VECTOR4F,

            UPDATE_INDEX_DATA_BUFFER,
            UPDATE_VERTEX_DATA_BUFFER
        };

        void setState(UInt32 state);

    private:
        void createGeometry(ramses::Effect& effect, UInt32 state);
        ramses::Effect* createEffect(UInt32 state);

        void createIndexDataBufferUInt16();
        void createIndexDataBufferUInt32();

        void createVertexDataBufferFloat();
        void createVertexDataBufferVector2F();
        void createVertexDataBufferVector3F();
        void createVertexDataBufferVector4F();
        void createVertexArrayBufferVector4F();

        ramses::MeshNode* m_meshNode;
        ramses::Effect& m_effect;
        ramses::GeometryBinding* m_geometry = nullptr;
        ramses::IndexDataBuffer* m_indexDataBufferUInt32 = nullptr;
        ramses::VertexDataBuffer* m_vertexDataBufferVec4 = nullptr;
    };
}

#endif
