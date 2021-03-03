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

namespace ramses_internal
{
    EffectResource::EffectResource(const String& vertexShader, const String& fragmentShader, const String& geometryShader,
        absl::optional<EDrawMode> geometryShaderInputType, const EffectInputInformationVector& uniformInputs,
        const EffectInputInformationVector& attributeInputs, const String& name, ResourceCacheFlag cacheFlag)
        : ResourceBase(EResourceType_Effect, cacheFlag, name)
        , m_uniformInputs(uniformInputs)
        , m_attributeInputs(attributeInputs)
        , m_fragmentShaderOffset(static_cast<UInt32>(vertexShader.size() + 1))
        , m_geometryShaderOffset(m_fragmentShaderOffset+static_cast<UInt32>(fragmentShader.size() + 1))
        , m_geometryShaderInputType(geometryShaderInputType)
    {
        const UInt32 dataLength = static_cast<UInt32>(vertexShader.size() + 1 + fragmentShader.size() + 1 + geometryShader.size() + 1); // including 3x terminating '0'
        ResourceBlob blob(dataLength);
        std::memcpy(blob.data(), vertexShader.c_str(), vertexShader.size() + 1);
        std::memcpy(blob.data() + m_fragmentShaderOffset, fragmentShader.c_str(), fragmentShader.size() + 1);
        std::memcpy(blob.data() + m_geometryShaderOffset, geometryShader.c_str(), geometryShader.size() + 1);
        setResourceData(std::move(blob));
    }

    EffectResource::EffectResource(const EffectInputInformationVector& uniformInputs, const EffectInputInformationVector& attributeInputs, absl::optional<EDrawMode> geometryShaderInputType,
        const String& name, UInt32 fragmentShaderOffset, UInt32 geometryShaderOffset, ResourceCacheFlag cacheFlag)
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
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data;
    }

    const char* EffectResource::getFragmentShader() const
    {
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_fragmentShaderOffset;
    }

    const char* EffectResource::getGeometryShader() const
    {
        const char* data = reinterpret_cast<const char*>(getResourceData().data());
        return data + m_geometryShaderOffset;
    }

    absl::optional<EDrawMode> EffectResource::getGeometryShaderInputType() const
    {
        // TODO (backported) once 27 merged to master, consider if we want to add few asserts in this class that
        // geometry shader input is not nullopt IFF geometry shader is set (for additional safety)
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

    DataFieldHandle EffectResource::getUniformDataFieldHandleByName(const String& name) const
    {
        for (UInt i = 0; i < m_uniformInputs.size(); ++i)
        {
            if (m_uniformInputs[i].inputName == name)
            {
                return DataFieldHandle(static_cast<UInt32>(i));
            }
        }
        return DataFieldHandle::Invalid();
    }

    DataFieldHandle EffectResource::getAttributeDataFieldHandleByName(const String& name) const
    {
        for (UInt i = 0; i < m_attributeInputs.size(); ++i)
        {
            if (m_attributeInputs[i].inputName == name)
            {
                return DataFieldHandle(static_cast<UInt32>(i));
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

        // TODO (backported) enable this once 27 merged to master
        // Also drag in the geometry shader offset inside the if below
        //if (m_geometryShaderInputType)
        //{
        //    output << static_cast<uint8_t>(*m_geometryShaderInputType);
        //}
        //else
        //{
        //    output << static_cast<uint8_t>(EDrawMode::NUMBER_OF_ELEMENTS);
        //}
    }

    IResource* EffectResource::CreateResourceFromMetadataStream(IInputStream& input, ResourceCacheFlag cacheFlag, const String& name)
    {
        EffectInputInformationVector uniformInputs;
        ReadInputVector(input, uniformInputs);
        EffectInputInformationVector attributeInputs;
        ReadInputVector(input, attributeInputs);

        UInt32 fragementShaderOffset = 0;
        input >> fragementShaderOffset;
        UInt32 geometryShaderOffset = 0;
        input >> geometryShaderOffset;

        absl::optional<EDrawMode> geometryShaderInputTypeOpt;
        // TODO (backported) enable this once 27 merged to master
        //uint8_t geometryShaderInputTypeInt = 0;
        //input >> geometryShaderInputTypeInt;
        //if (geometryShaderInputTypeInt != static_cast<uint8_t>(EDrawMode::NUMBER_OF_ELEMENTS))
        //{
        //    geometryShaderInputTypeOpt = static_cast<EDrawMode>(geometryShaderInputTypeInt);
        //}

        return new EffectResource(uniformInputs, attributeInputs, geometryShaderInputTypeOpt, name, fragementShaderOffset, geometryShaderOffset, cacheFlag);
    }

    void EffectResource::WriteInputVector(IOutputStream& stream, const EffectInputInformationVector& inputVector)
    {
        UInt32 length = static_cast<UInt32>(inputVector.size());
        stream << length;
        for (UInt32 i = 0; i < length; ++i)
        {
            const EffectInputInformation& input = inputVector[i];
            stream << input.inputName << input.elementCount << static_cast<UInt32>(input.dataType) << static_cast<UInt32>(input.semantics);
        }
    }

    void EffectResource::ReadInputVector(IInputStream& stream, EffectInputInformationVector& inputVector)
    {
        UInt32 length;
        stream >> length;
        inputVector.reserve(length);
        for (UInt32 i = 0; i < length; ++i)
        {
            EffectInputInformation input;
            UInt32 typeTmp;
            UInt32 semanticTmp;
            stream >> input.inputName >> input.elementCount >> typeTmp >> semanticTmp;
            input.dataType = static_cast<EDataType>(typeTmp);
            input.semantics = static_cast<EFixedSemantics>(semanticTmp);
            inputVector.push_back(input);
        }
    }
}
