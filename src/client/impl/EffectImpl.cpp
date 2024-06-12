//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/EffectImpl.h"
#include "impl/EffectInputImpl.h"
#include "impl/SerializationContext.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/EffectInputSemanticUtils.h"
#include "impl/ErrorReporting.h"

#include "internal/Components/ManagedResource.h"
#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/Components/ResourceHashUsage.h"
#include "internal/SceneGraph/Resource/EffectResource.h"

#include "fmt/format.h"

#include <cassert>

namespace ramses::internal
{
    EffectImpl::EffectImpl(ResourceHashUsage hashUsage, SceneImpl& scene, std::string_view effectname)
        : ResourceImpl(ERamsesObjectType::Effect, std::move(hashUsage), scene, effectname)
    {
    }

    EffectImpl::~EffectImpl() = default;

    void EffectImpl::deinitializeFrameworkData()
    {
        ResourceImpl::deinitializeFrameworkData();
        m_shaderWarnings.reset();
    }

    void EffectImpl::initializeFromFrameworkData(const EffectInputInformationVector& uniformInputs, const EffectInputInformationVector& attributeInputs, std::optional<EDrawMode> geometryShaderInputType)
    {
        m_effectUniformInputs = uniformInputs;
        m_effectAttributeInputs = attributeInputs;
        m_geometryShaderInputType = geometryShaderInputType;

        m_shaderWarnings.reset();
    }

    bool EffectImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!ResourceImpl::serialize(outStream, serializationContext))
            return false;

        const bool serializeUBO = (getClientImpl().getFramework().getFeatureLevel() >= EFeatureLevel_02);

        outStream << static_cast<uint32_t>(m_effectUniformInputs.size());
        for(const auto& input : m_effectUniformInputs)
        {
            outStream << input.inputName;
            outStream << static_cast<uint32_t>(input.dataType);
            outStream << static_cast<uint32_t>(input.elementCount);
            outStream << static_cast<uint32_t>(input.semantics);

            if (serializeUBO)
            {
                outStream << input.uniformBufferBinding.getValue();
                outStream << input.uniformBufferFieldOffset.getValue();
                outStream << input.uniformBufferFieldOffset.getValue();
            }
        }

        outStream << static_cast<uint32_t>(m_effectAttributeInputs.size());
        for (const auto& input : m_effectAttributeInputs)
        {
            outStream << input.inputName;
            outStream << static_cast<uint32_t>(input.dataType);
            outStream << static_cast<uint32_t>(input.semantics);
        }

        const int32_t gsInputType = (m_geometryShaderInputType ? static_cast<int32_t>(*m_geometryShaderInputType) : -1);
        outStream << gsInputType;

        return true;
    }

    bool EffectImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!ResourceImpl::deserialize(inStream, serializationContext))
            return false;

        const bool deserializeUBO = (getClientImpl().getFramework().getFeatureLevel() >= EFeatureLevel_02);

        uint32_t count = 0u;
        inStream >> count;
        m_effectUniformInputs.resize(count);
        for (uint32_t i = 0u; i < count; ++i)
        {
            inStream >> m_effectUniformInputs[i].inputName;

            uint32_t dataTypeAsUInt = 0;
            inStream >> dataTypeAsUInt;
            uint32_t elementCount = 0;
            inStream >> elementCount;
            uint32_t semanticAsUInt = 0;
            inStream >> semanticAsUInt;

            m_effectUniformInputs[i].dataType = static_cast<ramses::internal::EDataType>(dataTypeAsUInt);
            m_effectUniformInputs[i].elementCount = elementCount;
            m_effectUniformInputs[i].semantics = static_cast<EFixedSemantics>(semanticAsUInt);

            if (deserializeUBO)
            {
                uint32_t uniformBufferBindingAsUInt = 0;
                inStream >> uniformBufferBindingAsUInt;
                uint32_t uniformBufferElementSizeAsUInt = 0;
                inStream >> uniformBufferElementSizeAsUInt;
                uint32_t uniformBufferFieldOffsetAsUInt = 0;
                inStream >> uniformBufferFieldOffsetAsUInt;
                m_effectUniformInputs[i].uniformBufferBinding.getReference() = uniformBufferBindingAsUInt;
                m_effectUniformInputs[i].uniformBufferElementSize.getReference() = uniformBufferElementSizeAsUInt;
                m_effectUniformInputs[i].uniformBufferFieldOffset.getReference() = uniformBufferFieldOffsetAsUInt;
            }
        }

        inStream >> count;
        m_effectAttributeInputs.resize(count);
        for (uint32_t i = 0u; i < count; ++i)
        {
            inStream >> m_effectAttributeInputs[i].inputName;

            uint32_t dataTypeAsUInt = 0;
            inStream >> dataTypeAsUInt;
            uint32_t semanticAsUInt = 0;
            inStream >> semanticAsUInt;

            m_effectAttributeInputs[i].dataType = static_cast<ramses::internal::EDataType>(dataTypeAsUInt);
            m_effectAttributeInputs[i].elementCount = 1u;
            m_effectAttributeInputs[i].semantics = static_cast<EFixedSemantics>(semanticAsUInt);
        }

        int32_t gsInputType = -1;
        inStream >> gsInputType;
        if (gsInputType >= 0)
            m_geometryShaderInputType = static_cast<EDrawMode>(gsInputType);

        m_shaderWarnings.reset();

        DataLayoutCreationHelper::SetDataFieldMappingForUniformInputs(m_effectUniformInputs);
        DataLayoutCreationHelper::SetDataFieldMappingForAttributeInputs(m_effectAttributeInputs);

        return true;
    }

    void EffectImpl::onValidate(ValidationReportImpl& report) const
    {
        ResourceImpl::onValidate(report);
        if (!m_shaderWarnings.has_value())
        {
            const auto resourceHash{getLowlevelResourceHash()};
            assert(resourceHash.isValid());

            auto managedResource{getClientImpl().getResource(resourceHash)};
            if (!managedResource && !(managedResource = getClientImpl().getClientApplication().loadResource(resourceHash)))
            {
                report.add(EIssueType::Error, fmt::format("Unable to retrieve a resource by resource hash: {}", resourceHash), &getRamsesObject());
                return;
            }

            const EffectResource& effectResource{*managedResource->convertTo<EffectResource>()};
            if (!effectResource.isDeCompressedAvailable())
            {
                if (effectResource.isCompressedAvailable())
                {
                    effectResource.decompress();
                }
                else
                {
                    report.add(EIssueType::Error, fmt::format("EffectResource without compressed/decompressed data: {}", resourceHash), &getRamsesObject());
                    return;
                }
            }

            const GlslParser parser{effectResource.getVertexShader(), effectResource.getFragmentShader(), effectResource.getGeometryShader()};
            if (!parser.valid())
            {
                report.add(EIssueType::Error, fmt::format("Can't parse shaders provided by EffectResource: {}", resourceHash), &getRamsesObject());
                return;
            }

            m_shaderWarnings = parser.generateWarnings();
        }

        for (auto& w : *m_shaderWarnings)
        {
            report.add(EIssueType::Warning, fmt::format("{}: {}", w.stage, w.msg), &getRamsesObject());
        }
    }

    size_t EffectImpl::getUniformInputCount() const
    {
        return m_effectUniformInputs.size();
    }

    size_t EffectImpl::getAttributeInputCount() const
    {
        return m_effectAttributeInputs.size();
    }

    std::optional<UniformInput> EffectImpl::getUniformInput(size_t index) const
    {
        if (index >= getUniformInputCount())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: getUniformInput failed, index out of range!");
            return std::nullopt;
        }

        UniformInput input;
        initializeEffectInputData(*input.m_impl, m_effectUniformInputs[index], index);

        return input;
    }

    std::optional<AttributeInput> EffectImpl::getAttributeInput(size_t index) const
    {
        if (index >= getAttributeInputCount())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: getAttributeInput failed, index out of range!");
            return std::nullopt;
        }

        AttributeInput input;
        initializeEffectInputData(*input.m_impl, m_effectAttributeInputs[index], index);

        return input;
    }

    std::optional<UniformInput> EffectImpl::findUniformInput(std::string_view inputName) const
    {
        const size_t index = GetEffectInputIndex(m_effectUniformInputs, inputName);
        if (index == InvalidInputIndex)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: findUniformInput failed, uniform input '{}' could not be found in effect '{}'", inputName, getName());
            return std::nullopt;
        }

        UniformInput input;
        initializeEffectInputData(*input.m_impl, m_effectUniformInputs[index], index);

        return input;
    }

    std::optional<AttributeInput> EffectImpl::findAttributeInput(std::string_view inputName) const
    {
        const size_t index = GetEffectInputIndex(m_effectAttributeInputs, inputName);
        if (index == InvalidInputIndex)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: findAttributeInput failed, attribute input '{}' could not be found in effect '{}'", inputName, getName());
            return std::nullopt;
        }

        AttributeInput input;
        initializeEffectInputData(*input.m_impl, m_effectAttributeInputs[index], index);

        return input;
    }

    std::optional<UniformInput> EffectImpl::findUniformInput(EEffectUniformSemantic uniformSemantic) const
    {
        const size_t index = FindEffectInputIndex(m_effectUniformInputs, EffectInputSemanticUtils::GetEffectInputSemanticInternal(uniformSemantic));
        if (index == InvalidInputIndex)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: findUniformInput failed, semantic is not defined in effect!");
            return std::nullopt;
        }

        UniformInput input;
        initializeEffectInputData(*input.m_impl, m_effectUniformInputs[index], index);

        return input;
    }

    std::optional<AttributeInput> EffectImpl::findAttributeInput(EEffectAttributeSemantic attributeSemantic) const
    {
        const size_t index = FindEffectInputIndex(m_effectAttributeInputs, EffectInputSemanticUtils::GetEffectInputSemanticInternal(attributeSemantic));
        if (index == InvalidInputIndex)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: findAttributeInput failed, semantic not defined in effect!");
            return std::nullopt;
        }

        AttributeInput input;
        initializeEffectInputData(*input.m_impl, m_effectAttributeInputs[index], index);

        return input;
    }

    std::optional<ramses::UniformInput> EffectImpl::findUniformInputAtBinding(uint32_t uniformBufferBinding) const
    {
        const size_t index = GetUniformBufferInputIndex(m_effectUniformInputs, UniformBufferBinding{ uniformBufferBinding });
        if (index == InvalidInputIndex)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Effect: findUniformInputAtBinding failed, uniform buffer with specified binding is not defined in effect!");
            return std::nullopt;
        }

        UniformInput input;
        initializeEffectInputData(*input.m_impl, m_effectUniformInputs[index], index);

        return input;
    }

    const EffectInputInformationVector& EffectImpl::getUniformInputInformation() const
    {
        return m_effectUniformInputs;
    }

    const EffectInputInformationVector& EffectImpl::getAttributeInputInformation() const
    {
        return m_effectAttributeInputs;
    }

    size_t EffectImpl::GetEffectInputIndex(const EffectInputInformationVector& effectInputVector, std::string_view inputName)
    {
        const size_t numInputs = effectInputVector.size();
        for (size_t i = 0u; i < numInputs; ++i)
        {
            const EffectInputInformation& effectInputInfo = effectInputVector[i];
            if (effectInputInfo.inputName == inputName)
            {
                return i;
            }
        }

        return InvalidInputIndex;
    }

    size_t EffectImpl::GetUniformBufferInputIndex(const EffectInputInformationVector& effectInputVector, UniformBufferBinding uniformBufferBinding)
    {
        const size_t numInputs = effectInputVector.size();
        for (size_t i = 0u; i < numInputs; ++i)
        {
            const EffectInputInformation& effectInputInfo = effectInputVector[i];
            if (effectInputInfo.dataType == EDataType::UniformBuffer &&
                effectInputInfo.uniformBufferBinding == uniformBufferBinding)
            {
                return i;
            }
        }

        return InvalidInputIndex;
    }

    size_t EffectImpl::FindEffectInputIndex(const EffectInputInformationVector& effectInputVector, EFixedSemantics inputSemantics)
    {
        if (EFixedSemantics::Invalid == inputSemantics)
            return InvalidInputIndex;

        const size_t numInputs = effectInputVector.size();
        for (size_t i = 0u; i < numInputs; ++i)
        {
            const EffectInputInformation& effectInputInfo = effectInputVector[i];
            if (effectInputInfo.semantics == inputSemantics)
            {
                return i;
            }
        }

        return InvalidInputIndex;
    }

    void EffectImpl::initializeEffectInputData(EffectInputImpl& effectInputImpl, const EffectInputInformation& effectInputInfo, size_t index) const
    {
        effectInputImpl.initialize(getLowlevelResourceHash(), effectInputInfo, index);
    }

    bool EffectImpl::hasGeometryShader() const
    {
        return m_geometryShaderInputType.has_value();
    }

    bool EffectImpl::getGeometryShaderInputType(EDrawMode& inputType) const
    {
        if (!hasGeometryShader())
        {
            getErrorReporting().set(fmt::format("Effect::getGeometryShaderInputType: failed, effect '{}' has no geometry shader attached to it!", getName()), *this);
            return false;
        }

        inputType = *m_geometryShaderInputType;
        return true;
    }
}
