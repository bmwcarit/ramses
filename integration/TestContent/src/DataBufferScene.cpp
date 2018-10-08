//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/DataBufferScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-utils.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "Math3d/Vector4.h"
#include "SceneAPI/EDataType.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/RamsesClient.h"

namespace ramses_internal
{

    DataBufferScene::DataBufferScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_effect(*createEffect(state))
    {
        ramses::Appearance* appearance = m_scene.createAppearance(m_effect);

        m_meshNode = m_scene.createMeshNode("mesh");
        addMeshNodeToDefaultRenderGroup(*m_meshNode);

        m_meshNode->setAppearance(*appearance);

        createGeometry(m_effect, state);
    }

    void DataBufferScene::setState(UInt32 state)
    {
        switch (state)
        {
        case UPDATE_INDEX_DATA_BUFFER:
        {
            assert(nullptr != m_indexDataBufferUInt32);
            static const UInt32 updatedIndicesData[] =
            {
                3, 0
            };

            m_indexDataBufferUInt32->setData(reinterpret_cast<const char*>(updatedIndicesData), sizeof(updatedIndicesData));
            m_scene.flush();
            break;
        }
        case UPDATE_VERTEX_DATA_BUFFER:
        {
            assert(nullptr != m_vertexDataBufferVec4);

            static const float updatedVertices[] =
            {
                3.f,  0.f,  0.f, 1.0f,
                0.f,  0.f,  0.f, 1.0f,
                3.f, -3.f,  0.f, 1.0f,
            };
            m_vertexDataBufferVec4->setData(reinterpret_cast<const char*>(updatedVertices), sizeof(updatedVertices));
            m_scene.flush();
            break;
        }
        case VERTEX_ARRAY_BUFFER_VECTOR4F:
        {
            createVertexArrayBufferVector4F();
            m_scene.flush();
            break;
        }
        case VERTEX_DATA_BUFFER_VECTOR4F:
        {
            createVertexDataBufferVector4F();
            m_scene.flush();
            break;

        }
        default:
            assert(false);
        }
    }

    ramses::Effect* DataBufferScene::createEffect(UInt32 state)
    {
        switch (state)
        {
        case VERTEX_DATA_BUFFER_FLOAT:
            return getTestEffect("ramses-test-client-data-buffers-float");
        case VERTEX_DATA_BUFFER_VECTOR2F:
            return getTestEffect("ramses-test-client-data-buffers-vec2");
        case INDEX_DATA_BUFFER_UINT16:
        case VERTEX_DATA_BUFFER_VECTOR3F:
            return getTestEffect("ramses-test-client-data-buffers-vec3");
        case INDEX_DATA_BUFFER_UINT32:
        case VERTEX_DATA_BUFFER_VECTOR4F:
        case VERTEX_ARRAY_BUFFER_VECTOR4F:
            return getTestEffect("ramses-test-client-data-buffers-vec4");
        default:
            assert(false);
        }

        return nullptr;
    }

    void DataBufferScene::createGeometry(ramses::Effect& effect, UInt32 state)
    {
        m_geometry = m_scene.createGeometryBinding(effect);

        switch (state)
        {
        case INDEX_DATA_BUFFER_UINT16:
            createIndexDataBufferUInt16();
            createVertexDataBufferVector3F();
            break;
        case INDEX_DATA_BUFFER_UINT32:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector4F();
            break;
        case VERTEX_DATA_BUFFER_FLOAT:
            createIndexDataBufferUInt32();
            createVertexDataBufferFloat();
            break;
        case VERTEX_DATA_BUFFER_VECTOR2F:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector2F();
            break;
        case VERTEX_DATA_BUFFER_VECTOR3F:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector3F();
            break;
        case VERTEX_DATA_BUFFER_VECTOR4F:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector4F();
            break;
        case VERTEX_ARRAY_BUFFER_VECTOR4F:
            createIndexDataBufferUInt32();
            createVertexArrayBufferVector4F();
            break;
        default:
            assert(false);
        }


        m_meshNode->setGeometryBinding(*m_geometry);

        m_meshNode->setIndexCount(3);
    }

    void DataBufferScene::createIndexDataBufferUInt16()
    {
        static const UInt16 indicesData[] =
        {
            0, 1, 2
        };

        ramses::IndexDataBuffer* dataBuffer = m_scene.createIndexDataBuffer(sizeof(indicesData), ramses::EDataType_UInt16);
        dataBuffer->setData(reinterpret_cast<const char*>(indicesData), sizeof(indicesData));
        m_geometry->setIndices(*dataBuffer);
    }

    void DataBufferScene::createIndexDataBufferUInt32()
    {
        static const UInt32 indicesData[] =
        {
            0, 1, 2
        };

        ramses::IndexDataBuffer* dataBuffer = m_scene.createIndexDataBuffer(sizeof(indicesData), ramses::EDataType_UInt32);
        dataBuffer->setData(reinterpret_cast<const char*>(indicesData), sizeof(indicesData));
        m_geometry->setIndices(*dataBuffer);

        m_indexDataBufferUInt32 = dataBuffer;
    }

    void DataBufferScene::createVertexDataBufferFloat()
    {
        static const float verticesX[] =
        {
            0.f,
            0.f,
            3.f,
            3.f
        };

        static const float verticesY[] =
        {
            0.f,
            -3.f,
            -3.f,
            0.f
        };

        ramses::VertexDataBuffer* dataBufferX = m_scene.createVertexDataBuffer(sizeof(verticesX), ramses::EDataType_Float);
        dataBufferX->setData(reinterpret_cast<const char*>(verticesX), sizeof(verticesX));
        ramses::VertexDataBuffer* dataBufferY = m_scene.createVertexDataBuffer(sizeof(verticesY), ramses::EDataType_Float);
        dataBufferY->setData(reinterpret_cast<const char*>(verticesY), sizeof(verticesY));

        ramses::AttributeInput inputX;
        m_effect.findAttributeInput("a_positionX", inputX);
        m_geometry->setInputBuffer(inputX, *dataBufferX);

        ramses::AttributeInput inputY;
        m_effect.findAttributeInput("a_positionY", inputY);
        m_geometry->setInputBuffer(inputY, *dataBufferY);
    }

    void DataBufferScene::createVertexDataBufferVector2F()
    {
        static const float vertices[] =
        {
            0.f,  0.f,
            0.f, -3.f,
            3.f, -3.f,
            3.f,  0.f
        };

        ramses::VertexDataBuffer* dataBuffer = m_scene.createVertexDataBuffer(sizeof(vertices), ramses::EDataType_Vector2F);
        dataBuffer->setData(reinterpret_cast<const char*>(vertices), sizeof(vertices));

        ramses::AttributeInput input;
        m_effect.findAttributeInput("a_position", input);
        m_geometry->setInputBuffer(input, *dataBuffer);
    }

    void DataBufferScene::createVertexDataBufferVector3F()
    {
        static const float vertices[] =
        {
            0.f,  0.f,  0.f,
            0.f, -3.f,  0.f,
            3.f, -3.f,  0.f,
            3.f,  0.f,  0.f
        };

        ramses::VertexDataBuffer* dataBuffer = m_scene.createVertexDataBuffer(sizeof(vertices), ramses::EDataType_Vector3F);
        dataBuffer->setData(reinterpret_cast<const char*>(vertices), sizeof(vertices));

        ramses::AttributeInput input;
        m_effect.findAttributeInput("a_position", input);
        m_geometry->setInputBuffer(input, *dataBuffer);
    }

    void DataBufferScene::createVertexDataBufferVector4F()
    {
        static const float vertices[] =
        {
            0.f,  0.f,  0.f, 1.0f,
            0.f, -3.f,  0.f, 1.0f,
            3.f, -3.f,  0.f, 1.0f,
            3.f,  0.f,  0.f, 1.0f
        };

        ramses::VertexDataBuffer* dataBuffer = m_scene.createVertexDataBuffer(sizeof(vertices), ramses::EDataType_Vector4F);
        dataBuffer->setData(reinterpret_cast<const char*>(vertices), sizeof(vertices));

        ramses::AttributeInput input;
        m_effect.findAttributeInput("a_position", input);
        m_geometry->setInputBuffer(input, *dataBuffer);

        m_vertexDataBufferVec4 = dataBuffer;
    }

    void DataBufferScene::createVertexArrayBufferVector4F()
    {
        static const float vertices[] =
        {
            3.f,  0.f,  0.f, 1.0f,
            0.f,  0.f,  0.f, 1.0f,
            1.5f, -3.f,  0.f, 1.0f
        };

        const ramses::Vector4fArray* arrayBuffer = m_client.createConstVector4fArray(3u, vertices);

        ramses::AttributeInput input;
        m_effect.findAttributeInput("a_position", input);
        m_geometry->setInputBuffer(input, *arrayBuffer);
    }
}
