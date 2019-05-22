//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/WarpingPass.h"

#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "RendererLib/WarpingMeshData.h"

namespace ramses_internal
{
    WarpingPass::WarpingPass(IDevice& device, const WarpingMeshData& warpingMeshData)
        : m_indexCount(static_cast<UInt32>(warpingMeshData.getIndices().size()))
        , m_device(device)
    {
        initEffect();
        initGeometry(warpingMeshData);
    }

    WarpingPass::~WarpingPass()
    {
        m_device.deleteVertexBuffer(m_texcoordBufferResource);
        m_device.deleteIndexBuffer(m_indexBufferResource);
        m_device.deleteVertexBuffer(m_vertexBufferResource);
        m_device.deleteShader(m_shaderResource);
    }

    void WarpingPass::initGeometry(const WarpingMeshData& warpingMeshData)
    {
        const UInt32 vertexCount = static_cast<UInt32>(warpingMeshData.getVertexPositions().size());
        assert(warpingMeshData.getTextureCoordinates().size() == vertexCount);

        const ArrayResource vertexArrayRes(EResourceType_VertexArray, vertexCount, EDataType_Vector3F, reinterpret_cast<const Byte*>(&warpingMeshData.getVertexPositions()[0]), ResourceCacheFlag_DoNotCache, String());
        m_vertexBufferResource = m_device.allocateVertexBuffer(vertexArrayRes.getElementType(), vertexArrayRes.getDecompressedDataSize());
        assert(m_vertexBufferResource.isValid());
        m_device.uploadVertexBufferData(m_vertexBufferResource, vertexArrayRes.getResourceData()->getRawData(), vertexArrayRes.getDecompressedDataSize());

        const ArrayResource texcoordArrayRes(EResourceType_VertexArray, vertexCount, EDataType_Vector2F, reinterpret_cast<const Byte*>(&warpingMeshData.getTextureCoordinates()[0]), ResourceCacheFlag_DoNotCache, String());
        m_texcoordBufferResource = m_device.allocateVertexBuffer(texcoordArrayRes.getElementType(), texcoordArrayRes.getDecompressedDataSize());
        assert(m_texcoordBufferResource.isValid());
        m_device.uploadVertexBufferData(m_texcoordBufferResource, texcoordArrayRes.getResourceData()->getRawData(), texcoordArrayRes.getDecompressedDataSize());

        assert(0 != m_indexCount);
        const ArrayResource indexArrayRes(EResourceType_IndexArray, m_indexCount, EDataType_UInt16, reinterpret_cast<const Byte*>(&warpingMeshData.getIndices()[0]), ResourceCacheFlag_DoNotCache, String());
        m_indexBufferResource = m_device.allocateIndexBuffer(indexArrayRes.getElementType(), indexArrayRes.getDecompressedDataSize());
        assert(m_indexBufferResource.isValid());
        m_device.uploadIndexBufferData(m_indexBufferResource, indexArrayRes.getResourceData()->getRawData(), indexArrayRes.getDecompressedDataSize());
    }

    void WarpingPass::initEffect()
    {
        static const char* vertexShader =
            "#version 100\n"
            "precision highp float;\n"
            "attribute vec3 a_position; \n"
            "attribute vec2 a_texcoord; \n"
            "\n"
            "varying vec2 v_texcoord; \n"
            "\n"
            "void main()\n"
            "{\n"
            "  v_texcoord = a_texcoord; \n"
            "  gl_Position = vec4(a_position, 1.0); \n"
            "}\n";

        static const char* fragmentShader =
            "#version 100\n"
            "precision highp float;\n"
            "uniform sampler2D u_texture; \n"
            "varying vec2 v_texcoord; \n"
            "\n"
            "void main(void)\n"
            "{\n"
            "  gl_FragColor = texture2D(u_texture, v_texcoord); \n"
            "}\n";

        // construct effect
        EffectInputInformation a_position;
        a_position.inputName = "a_position";
        a_position.dataType = EDataType_Vector3Buffer;
        a_position.semantics = EFixedSemantics_VertexPositionAttribute;

        EffectInputInformation a_texcoord;
        a_texcoord.inputName = "a_texcoord";
        a_texcoord.dataType = EDataType_Vector2Buffer;
        a_texcoord.semantics = EFixedSemantics_Invalid;

        EffectInputInformationVector attributeInputs;
        attributeInputs.push_back(a_position);
        attributeInputs.push_back(a_texcoord);

        EffectInputInformation u_texture;
        u_texture.inputName = "u_texture";
        u_texture.dataType = EDataType_TextureSampler;
        u_texture.semantics = EFixedSemantics_Invalid;
        u_texture.textureType = EEffectInputTextureType_Texture2D;

        EffectInputInformationVector uniformInputs;
        uniformInputs.push_back(u_texture);

        EffectResource effect(vertexShader, fragmentShader, uniformInputs, attributeInputs, "WarpingEffect", ResourceCacheFlag_DoNotCache);

        // store field handles
        m_vertexPositionField = effect.getAttributeDataFieldHandleByName("a_position");
        m_inputRenderBufferField = effect.getUniformDataFieldHandleByName("u_texture");
        m_texcoordField = effect.getAttributeDataFieldHandleByName("a_texcoord");

        assert(m_vertexPositionField.isValid());
        assert(m_inputRenderBufferField.isValid());
        assert(m_texcoordField.isValid());

        // upload
        m_shaderResource = m_device.uploadShader(effect);
        assert(m_shaderResource.isValid());
    }

    void WarpingPass::execute(DeviceResourceHandle sourceColorBuffer)
    {
        m_device.activateShader(m_shaderResource);

        m_device.activateTexture(sourceColorBuffer, m_inputRenderBufferField);
        const UInt32 isotropicFilteringLevel = 1u;
        m_device.setTextureSampling(m_inputRenderBufferField,
            EWrapMethod::Clamp, EWrapMethod::Clamp, EWrapMethod::Clamp, ESamplingMethod::Linear, ESamplingMethod::Linear, isotropicFilteringLevel);

        m_device.activateIndexBuffer(m_indexBufferResource);
        m_device.activateVertexBuffer(m_vertexBufferResource, m_vertexPositionField, 0u);
        m_device.activateVertexBuffer(m_texcoordBufferResource, m_texcoordField, 0u);
        m_device.drawIndexedTriangles(0, m_indexCount, 1u);
    }
}
