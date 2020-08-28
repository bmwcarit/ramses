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
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "Math3d/Vector4.h"
#include "SceneAPI/EDataType.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/AttributeInput.h"

namespace ramses_internal
{

    DataBufferScene::DataBufferScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
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

            m_indexDataBufferUInt32->updateData(0u, 2u, updatedIndicesData);
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
            m_vertexDataBufferVec4->updateData(0u, 3u, updatedVertices);
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

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::UInt16, 3u);
        dataBuffer->updateData(0u, 3u, indicesData);
        m_geometry->setIndices(*dataBuffer);
    }

    void DataBufferScene::createIndexDataBufferUInt32()
    {
        static const UInt32 indicesData[] =
        {
            0, 1, 2
        };

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::UInt32, 3u);
        dataBuffer->updateData(0u, 3u, indicesData);
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

        ramses::ArrayBuffer* dataBufferX = m_scene.createArrayBuffer(ramses::EDataType::Float, 4u);
        dataBufferX->updateData(0u, 4u, verticesX);
        ramses::ArrayBuffer* dataBufferY = m_scene.createArrayBuffer(ramses::EDataType::Float, 4u);
        dataBufferY->updateData(0u, 4u, verticesY);

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

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector2F, 8u);
        dataBuffer->updateData(0u, 4u, vertices);

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

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 12u);
        dataBuffer->updateData(0u, 4u, vertices);

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

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector4F, 16u);
        dataBuffer->updateData(0u, 4u, vertices);

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

        const ramses::ArrayResource* arrayBuffer = m_scene.createArrayResource(ramses::EDataType::Vector4F, 3u, vertices);

        ramses::AttributeInput input;
        m_effect.findAttributeInput("a_position", input);
        m_geometry->setInputBuffer(input, *arrayBuffer);
    }
}
