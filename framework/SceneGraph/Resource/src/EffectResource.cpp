//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Resource/EffectResource.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "AppearanceEnumsImpl.h"

namespace ramses_internal
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

    EffectResource::EffectResource(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader,
        std::optional<EDrawMode> geometryShaderInputType, const EffectInputInformationVector& uniformInputs,
        const EffectInputInformationVector& attributeInputs, std::string_view name, ResourceCacheFlag cacheFlag)
        : ResourceBase(EResourceType_Effect, cacheFlag, name)
        , m_uniformInputs(uniformInputs)
        , m_attributeInputs(attributeInputs)
        , m_fragmentShaderOffset(static_cast<uint32_t>(vertexShader.size() + 1))
        , m_geometryShaderOffset(m_fragmentShaderOffset+static_cast<uint32_t>(fragmentShader.size() + 1))
        , m_geometryShaderInputType(geometryShaderInputType)
    {
        assert((geometryShader.empty() != geometryShaderInputType.has_value()));
        const uint32_t dataLength = static_cast<uint32_t>(vertexShader.size() + 1 + fragmentShader.size() + 1 + geometryShader.size() + 1); // including 3x terminating '0'
        ResourceBlob blob(dataLength);
        std::memcpy(blob.data(), vertexShader.c_str(), vertexShader.size() + 1);
        std::memcpy(blob.data() + m_fragmentShaderOffset, fragmentShader.c_str(), fragmentShader.size() + 1);
        std::memcpy(blob.data() + m_geometryShaderOffset, geometryShader.c_str(), geometryShader.size() + 1);
        setResourceData(std::move(blob));
    }

    EffectResource::EffectResource(const EffectInputInformationVector& uniformInputs, const EffectInputInformationVector& attributeInputs, std::optional<EDrawMode> geometryShaderInputType,
        std::string_view name, uint32_t fragmentShaderOffset, uint32_t geometryShaderOffset, ResourceCacheFlag cacheFlag)
        : ResourceBase(EResourceType_Effect, cacheFlag, name)
        , m_uniformInputs(uniformInputs)
        , m_attributeInputs(attributeInputs)
        , m_fragmentShaderOffset(fragmentShaderOffset)
        , m_geometryShaderOffset(geometryShaderOffset)
        , m_geometryShaderInputType(geometryShaderInputType)
    {
    }

    const char* EffectResource::getVertexShader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get char* from blob
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data;
    }

    const char* EffectResource::getFragmentShader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get char* from blob
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_fragmentShaderOffset;
    }

    const char* EffectResource::getGeometryShader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get char* from blob
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_geometryShaderOffset;
    }

    std::optional<EDrawMode> EffectResource::getGeometryShaderInputType() const
    {
        return m_geometryShaderInputType;
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
        WriteInputVector(output, m_uniformInputs);
        WriteInputVector(output, m_attributeInputs);
        output << m_fragmentShaderOffset;
        output << m_geometryShaderOffset;
        if (m_geometryShaderInputType.has_value())
        {
            output << m_geometryShaderInputType.value();
        }
        else
        {
            constexpr auto maxValue = std::numeric_limits<std::underlying_type_t<EDrawMode>>::max();
            static_assert(ramses::DrawModeNames.size() < maxValue, "Last enum value is reserved");
            output << maxValue;
        }
    }

    std::unique_ptr<IResource> EffectResource::CreateResourceFromMetadataStream(IInputStream& input, ResourceCacheFlag cacheFlag, std::string_view name)
    {
        EffectInputInformationVector uniformInputs;
        ReadInputVector(input, uniformInputs);
        EffectInputInformationVector attributeInputs;
        ReadInputVector(input, attributeInputs);

        uint32_t fragementShaderOffset = 0;
        input >> fragementShaderOffset;
        uint32_t geometryShaderOffset = 0;
        input >> geometryShaderOffset;
        std::underlying_type_t<EDrawMode> gsInputTypeValue = 0;
        input >> gsInputTypeValue;

        std::optional<EDrawMode> gsInputType;
        if (IsValidDrawmode(gsInputTypeValue))
        {
            gsInputType = static_cast<EDrawMode>(gsInputTypeValue);
        }
        return std::unique_ptr<EffectResource>(new EffectResource(uniformInputs, attributeInputs, gsInputType, name, fragementShaderOffset, geometryShaderOffset, cacheFlag));
    }

    void EffectResource::WriteInputVector(IOutputStream& stream, const EffectInputInformationVector& inputVector)
    {
        uint32_t length = static_cast<uint32_t>(inputVector.size());
        stream << length;
        for (uint32_t i = 0; i < length; ++i)
        {
            const EffectInputInformation& input = inputVector[i];
            stream << input.inputName << input.elementCount << static_cast<uint32_t>(input.dataType) << static_cast<uint32_t>(input.semantics);
        }
    }

    void EffectResource::ReadInputVector(IInputStream& stream, EffectInputInformationVector& inputVector)
    {
        uint32_t length;
        stream >> length;
        inputVector.reserve(length);
        for (uint32_t i = 0; i < length; ++i)
        {
            EffectInputInformation input;
            uint32_t typeTmp;
            uint32_t semanticTmp;
            stream >> input.inputName >> input.elementCount >> typeTmp >> semanticTmp;
            input.dataType = static_cast<EDataType>(typeTmp);
            input.semantics = static_cast<EFixedSemantics>(semanticTmp);
            inputVector.push_back(input);
        }
    }
}
