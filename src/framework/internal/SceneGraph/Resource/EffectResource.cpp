//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/SceneGraph/SceneUtils/DataLayoutCreationHelper.h"
#include "impl/AppearanceEnumsImpl.h"
#include "absl/algorithm/container.h"

namespace ramses::internal
{
    namespace
    {
        constexpr bool IsValidDrawmode(std::underlying_type_t<EDrawMode> value)
        {
            switch (static_cast<EDrawMode>(value))
            {
            case EDrawMode::Points:
            case EDrawMode::Lines:
            case EDrawMode::LineLoop:
            case EDrawMode::Triangles:
            case EDrawMode::TriangleStrip:
            case EDrawMode::TriangleFan:
            case EDrawMode::LineStrip:
                return true;
            }
            return false;
        }
    }

    EffectResource::EffectResource(const std::string& vertexShader,
        const std::string& fragmentShader,
        const std::string& geometryShader,
        const SPIRVShaders& spirvShaders,
        std::optional<EDrawMode> geometryShaderInputType,
        EffectInputInformationVector uniformInputs,
        EffectInputInformationVector attributeInputs,
        std::string_view name,
        EFeatureLevel featureLevel)
        : ResourceBase(EResourceType::Effect, name)
        , m_uniformInputs(std::move(uniformInputs))
        , m_attributeInputs(std::move(attributeInputs))
        , m_byteOffsets(CreateByteOffsets(vertexShader, fragmentShader, geometryShader, spirvShaders))
        , m_geometryShaderInputType(geometryShaderInputType)
        , m_featureLevel{ featureLevel }
    {
        assert((geometryShader.empty() != geometryShaderInputType.has_value()));
        assert(absl::c_none_of(m_uniformInputs, [](const auto& input) { return input.elementCount == std::numeric_limits<uint32_t>::max(); }));
        assert(absl::c_none_of(m_attributeInputs, [](const auto& input) { return input.elementCount == std::numeric_limits<uint32_t>::max(); }));

        const auto dataLength = m_byteOffsets[EByteOffsetIndex_EndOfData];
        ResourceBlob blob(dataLength);
        std::memcpy(blob.data() + m_byteOffsets[EByteOffsetIndex_VertexShader], vertexShader.c_str(), vertexShader.size() + 1);
        std::memcpy(blob.data() + m_byteOffsets[EByteOffsetIndex_FragmentShader], fragmentShader.c_str(), fragmentShader.size() + 1);
        std::memcpy(blob.data() + m_byteOffsets[EByteOffsetIndex_GeometryShader], geometryShader.c_str(), geometryShader.size() + 1);

        if(getVertexShaderSPIRVSize() > 0u)
            std::memcpy(blob.data() + m_byteOffsets[EByteOffsetIndex_VertexSPIRV], spirvShaders.m_vertexSPIRVBlob.data(), getVertexShaderSPIRVSize());
        if(getFragmentShaderSPIRVSize() > 0u)
            std::memcpy(blob.data() + m_byteOffsets[EByteOffsetIndex_FragmentSPIRV], spirvShaders.m_fragmentSPIRVBlob.data(), getFragmentShaderSPIRVSize());
        if(getGeometryShaderSPIRVSize() > 0u)
            std::memcpy(blob.data() + m_byteOffsets[EByteOffsetIndex_GeometrySPIRV], spirvShaders.m_geometrySPIRVBlob.data(), getGeometryShaderSPIRVSize());

        setResourceData(std::move(blob));

        setDataFieldMappingForInputs();
    }

    EffectResource::EffectResource(EffectInputInformationVector&& uniformInputs,
        EffectInputInformationVector&& attributeInputs,
        std::optional<EDrawMode> geometryShaderInputType,
        std::string_view name,
        EffectByteOffsets&& byteOffsets,
        EFeatureLevel featureLevel)
        : ResourceBase(EResourceType::Effect, name)
        , m_uniformInputs(std::move(uniformInputs))
        , m_attributeInputs(std::move(attributeInputs))
        , m_byteOffsets(std::move(byteOffsets))
        , m_geometryShaderInputType(geometryShaderInputType)
        , m_featureLevel{ featureLevel }
    {
        setDataFieldMappingForInputs();
    }

    EffectResource::EffectByteOffsets EffectResource::CreateByteOffsets(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader, const SPIRVShaders& spirvShaders)
    {
        const auto vertSPIRVSize    = static_cast<uint32_t>(spirvShaders.m_vertexSPIRVBlob.size() * sizeof(uint32_t));
        const auto fragSPIRVSize    = static_cast<uint32_t>(spirvShaders.m_fragmentSPIRVBlob.size() * sizeof(uint32_t));
        const auto geomSPIRVSize    = static_cast<uint32_t>(spirvShaders.m_geometrySPIRVBlob.size() * sizeof(uint32_t));
        const auto vertShaderSize   = static_cast<uint32_t>(vertexShader.size() + 1u);
        const auto fragShaderSize   = static_cast<uint32_t>(fragmentShader.size() + 1u);
        const auto geomShaderSize   = static_cast<uint32_t>(geometryShader.size() + 1u);

        // Effect resource blob ends up having SPIRV (uint32_t) and Glsl shader string (char), which have
        // different alignment requirements. Care needs to be taken when if/when making changes or adding
        // so that data can be accessed with respect to alignment requirements

        EffectByteOffsets byteOffsets = { 0u };
        byteOffsets[EByteOffsetIndex_VertexSPIRV]       = 0u;
        byteOffsets[EByteOffsetIndex_FragmentSPIRV]     = vertSPIRVSize;
        byteOffsets[EByteOffsetIndex_GeometrySPIRV]     = byteOffsets[EByteOffsetIndex_FragmentSPIRV]   + fragSPIRVSize;
        byteOffsets[EByteOffsetIndex_VertexShader]      = byteOffsets[EByteOffsetIndex_GeometrySPIRV]   + geomSPIRVSize;
        byteOffsets[EByteOffsetIndex_FragmentShader]    = byteOffsets[EByteOffsetIndex_VertexShader]    + vertShaderSize;
        byteOffsets[EByteOffsetIndex_GeometryShader]    = byteOffsets[EByteOffsetIndex_FragmentShader]  + fragShaderSize;
        byteOffsets[EByteOffsetIndex_EndOfData]         = byteOffsets[EByteOffsetIndex_GeometryShader]  + geomShaderSize;

        return byteOffsets;
    }

    const char* EffectResource::getVertexShader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get char* from blob
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_byteOffsets[EByteOffsetIndex_VertexShader];;
    }

    const char* EffectResource::getFragmentShader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get char* from blob
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_byteOffsets[EByteOffsetIndex_FragmentShader];
    }

    const char* EffectResource::getGeometryShader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get char* from blob
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_byteOffsets[EByteOffsetIndex_GeometryShader];
    }

    std::optional<EDrawMode> EffectResource::getGeometryShaderInputType() const
    {
        return m_geometryShaderInputType;
    }

    uint32_t EffectResource::getVertexShaderSPIRVSize() const
    {
        return m_byteOffsets[EByteOffsetIndex_FragmentSPIRV] - m_byteOffsets[EByteOffsetIndex_VertexSPIRV];
    }

    uint32_t EffectResource::getFragmentShaderSPIRVSize() const
    {
        return m_byteOffsets[EByteOffsetIndex_GeometrySPIRV] - m_byteOffsets[EByteOffsetIndex_FragmentSPIRV];
    }

    uint32_t EffectResource::getGeometryShaderSPIRVSize() const
    {
        return m_byteOffsets[EByteOffsetIndex_VertexShader] - m_byteOffsets[EByteOffsetIndex_GeometrySPIRV];
    }

    const uint32_t* EffectResource::getVertexShaderSPIRV() const
    {
        assert(getVertexShaderSPIRVSize() > 0u);
        const auto* data = getResourceData().data();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const uint32_t*>(data + m_byteOffsets[EByteOffsetIndex_VertexSPIRV]);
    }

    const uint32_t* EffectResource::getFragmentShaderSPIRV() const
    {
        assert(getFragmentShaderSPIRVSize() > 0u);
        const auto* data = getResourceData().data();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const uint32_t*>(data + m_byteOffsets[EByteOffsetIndex_FragmentSPIRV]);
    }

    const uint32_t* EffectResource::getGeometryShaderSPIRV() const
    {
        assert(getGeometryShaderSPIRVSize() > 0u);
        const auto* data = getResourceData().data();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const uint32_t*>(data + m_byteOffsets[EByteOffsetIndex_GeometrySPIRV]);
    }

    const EffectInputInformationVector& EffectResource::getUniformInputs() const
    {
        return m_uniformInputs;
    }

    const EffectInputInformationVector& EffectResource::getAttributeInputs() const
    {
        return m_attributeInputs;
    }

    DataFieldHandle EffectResource::getUniformDataFieldHandleByName(std::string_view name) const
    {
        for (size_t i = 0; i < m_uniformInputs.size(); ++i)
        {
            if (m_uniformInputs[i].inputName == name)
            {
                return DataFieldHandle(static_cast<uint32_t>(i));
            }
        }
        return DataFieldHandle::Invalid();
    }

    DataFieldHandle EffectResource::getAttributeDataFieldHandleByName(std::string_view name) const
    {
        for (size_t i = 0; i < m_attributeInputs.size(); ++i)
        {
            if (m_attributeInputs[i].inputName == name)
            {
                return DataFieldHandle(static_cast<uint32_t>(i));
            }
        }
        return DataFieldHandle::Invalid();
    }

    void EffectResource::serializeResourceMetadataToStream(IOutputStream& output) const
    {
        WriteInputVector(output, m_uniformInputs, m_featureLevel);
        WriteInputVector(output, m_attributeInputs, m_featureLevel);
        output << m_byteOffsets[EByteOffsetIndex_FragmentShader];
        output << m_byteOffsets[EByteOffsetIndex_GeometryShader];

        if (m_geometryShaderInputType.has_value())
        {
            output << m_geometryShaderInputType.value();
        }
        else
        {
            constexpr auto maxValue = std::numeric_limits<std::underlying_type_t<EDrawMode>>::max();
            static_assert(DrawModeNames.size() < maxValue, "Last enum value is reserved");
            output << maxValue;
        }

        if (m_featureLevel >= EFeatureLevel_02)
        {
            output << m_byteOffsets[EByteOffsetIndex_VertexShader];
            output << m_byteOffsets[EByteOffsetIndex_VertexSPIRV];
            output << m_byteOffsets[EByteOffsetIndex_FragmentSPIRV];
            output << m_byteOffsets[EByteOffsetIndex_GeometrySPIRV];
            output << m_byteOffsets[EByteOffsetIndex_EndOfData];
        }
    }

    std::unique_ptr<IResource> EffectResource::CreateResourceFromMetadataStream(IInputStream& input, std::string_view name, EFeatureLevel featureLevel)
    {
        EffectInputInformationVector uniformInputs;
        ReadInputVector(input, uniformInputs, featureLevel);
        EffectInputInformationVector attributeInputs;
        ReadInputVector(input, attributeInputs, featureLevel);

        EffectByteOffsets byteOffsets = { 0u };
        input >> byteOffsets[EByteOffsetIndex_FragmentShader];
        input >> byteOffsets[EByteOffsetIndex_GeometryShader];
        std::underlying_type_t<EDrawMode> gsInputTypeValue = 0;
        input >> gsInputTypeValue;

        if (featureLevel >= EFeatureLevel_02)
        {
            input >> byteOffsets[EByteOffsetIndex_VertexShader];
            input >> byteOffsets[EByteOffsetIndex_VertexSPIRV];
            input >> byteOffsets[EByteOffsetIndex_FragmentSPIRV];
            input >> byteOffsets[EByteOffsetIndex_GeometrySPIRV];
            input >> byteOffsets[EByteOffsetIndex_EndOfData];
        }

        std::optional<EDrawMode> gsInputType;
        if (IsValidDrawmode(gsInputTypeValue))
        {
            gsInputType = static_cast<EDrawMode>(gsInputTypeValue);
        }
        return std::unique_ptr<EffectResource>(new EffectResource(std::move(uniformInputs), std::move(attributeInputs), gsInputType, name, std::move(byteOffsets), featureLevel));
    }

    void EffectResource::WriteInputVector(IOutputStream& stream, const EffectInputInformationVector& inputVector, EFeatureLevel featureLevel)
    {
        const bool serializeUBO = (featureLevel >= EFeatureLevel_02);
        const auto length = static_cast<uint32_t>(inputVector.size());
        stream << length;
        for (uint32_t i = 0; i < length; ++i)
        {
            const EffectInputInformation& input = inputVector[i];
            stream << input.inputName << input.elementCount << static_cast<uint32_t>(input.dataType) << static_cast<uint32_t>(input.semantics);
            if (serializeUBO)
                stream << input.uniformBufferBinding.getValue() << input.uniformBufferElementSize.getValue() << input.uniformBufferFieldOffset.getValue();
        }
    }

    void EffectResource::ReadInputVector(IInputStream& stream, EffectInputInformationVector& inputVector, EFeatureLevel featureLevel)
    {
        const bool deserializeUBO = (featureLevel >= EFeatureLevel_02);
        uint32_t length = 0;
        stream >> length;
        inputVector.reserve(length);
        for (uint32_t i = 0; i < length; ++i)
        {
            EffectInputInformation input;
            stream >> input.inputName >> input.elementCount;

            uint32_t tmpVal{};
            stream >> tmpVal;
            input.dataType = static_cast<EDataType>(tmpVal);
            stream >> tmpVal;
            input.semantics = static_cast<EFixedSemantics>(tmpVal);

            if (deserializeUBO)
                stream >> input.uniformBufferBinding.getReference() >> input.uniformBufferElementSize.getReference() >> input.uniformBufferFieldOffset.getReference();

            inputVector.push_back(input);
        }
    }

    void EffectResource::setDataFieldMappingForInputs()
    {
        DataLayoutCreationHelper::SetDataFieldMappingForUniformInputs(m_uniformInputs);
        DataLayoutCreationHelper::SetDataFieldMappingForAttributeInputs(m_attributeInputs);
    }
}
