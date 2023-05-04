//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYBUFFERSCENE_H
#define RAMSES_ARRAYBUFFERSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"
#include "Collections/Vector.h"

namespace ramses
{
    class Effect;
    class ArrayBuffer;
}

namespace ramses_internal
{
    class ArrayBufferScene : public IntegrationScene
    {
    public:
        ArrayBufferScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            INDEX_DATA_BUFFER_UINT16 = 0,
            INDEX_DATA_BUFFER_UINT32,

            VERTEX_DATA_BUFFER_FLOAT,
            VERTEX_DATA_BUFFER_VECTOR2F,
            VERTEX_DATA_BUFFER_VECTOR3F,
            VERTEX_DATA_BUFFER_VECTOR4F,
            VERTEX_ARRAY_RESOURCE_VECTOR4F,
            VERTEX_DATA_BUFFER_INTERLEAVED,
            VERTEX_DATA_BUFFER_INTERLEAVED_TWO_STRIDES,
            VERTEX_DATA_BUFFER_INTERLEAVED_SINGLE_ATTRIB,
            VERTEX_DATA_BUFFER_INTERLEAVED_START_VERTEX,

            UPDATE_INDEX_DATA_BUFFER,
            UPDATE_VERTEX_DATA_BUFFER,
            UPDATE_INTERLEAVED_VERTEX_DATA_BUFFER,
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
        void createVertexArrayResourceVector4F();
        void createVertexArrayBufferInterleaved();
        void createVertexArrayBufferInterleavedTwoStrides();
        void createVertexArrayBufferInterleavedSingleAttrib();
        void createVertexArrayBufferInterleavedStartVertex();

        ramses::MeshNode* m_meshNode;
        ramses::Effect& m_effect;
        ramses::GeometryBinding* m_geometry = nullptr;
        ramses::ArrayBuffer* m_indexDataBufferUInt32 = nullptr;
        ramses::ArrayBuffer* m_vertexDataBufferVec4 = nullptr;
        ramses::ArrayBuffer* m_vertexDataBufferInterleaved = nullptr;
    };
}

#endif
