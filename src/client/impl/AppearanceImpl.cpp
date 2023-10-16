//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// client API
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/Effect.h"
#include "ramses/client/DataObject.h"

// internal
#include "impl/AppearanceImpl.h"
#include "impl/EffectImpl.h"
#include "impl/EffectInputImpl.h"
#include "impl/ObjectIteratorImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/AppearanceUtils.h"
#include "impl/SerializationContext.h"
#include "impl/SceneImpl.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "impl/DataTypeUtils.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneUtils/DataLayoutCreationHelper.h"
#include "internal/SceneGraph/SceneUtils/ISceneDataArrayAccessor.h"
#include "internal/SceneGraph/SceneUtils/DataInstanceHelper.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include <algorithm>

namespace ramses::internal
{
    AppearanceImpl::AppearanceImpl(SceneImpl& scene, std::string_view appearancename)
        : SceneObjectImpl(scene, ERamsesObjectType::Appearance, appearancename)
        , m_effectImpl(nullptr)
    {
    }

    AppearanceImpl::~AppearanceImpl() = default;

    bool AppearanceImpl::setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        getIScene().setRenderStateBlendFactors(m_renderStateHandle, srcColor, destColor, srcAlpha, destAlpha);
        return true;
    }

    bool AppearanceImpl::getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const
    {
        const RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        srcColor  = rs.blendFactorSrcColor;
        destColor = rs.blendFactorDstColor;
        srcAlpha  = rs.blendFactorSrcAlpha;
        destAlpha = rs.blendFactorDstAlpha;
        return true;
    }

    bool AppearanceImpl::setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        if ((operationColor == EBlendOperation::Disabled) != (operationAlpha == EBlendOperation::Disabled))
        {
            getErrorReporting().set("Appearance::setBlendingOperations:  invalid combination - one of operationColor or operationAlpha disabled and the other not!");
            return false;
        }

        getIScene().setRenderStateBlendOperations(m_renderStateHandle, operationColor, operationAlpha);

        return true;
    }

    bool AppearanceImpl::getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const
    {
        const RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        operationColor = rs.blendOperationColor;
        operationAlpha = rs.blendOperationAlpha;
        return true;
    }

    bool AppearanceImpl::setBlendingColor(const vec4f& color)
    {
        getIScene().setRenderStateBlendColor(m_renderStateHandle, color);
        return true;
    }

    bool AppearanceImpl::getBlendingColor(vec4f& color) const
    {
        color = getIScene().getRenderState(m_renderStateHandle).blendColor;
        return true;
    }

    bool AppearanceImpl::setDepthFunction(EDepthFunc func)
    {
        getIScene().setRenderStateDepthFunc(m_renderStateHandle, func);
        return true;
    }

    bool AppearanceImpl::getDepthFunction(EDepthFunc& func) const
    {
        func = getIScene().getRenderState(m_renderStateHandle).depthFunc;
        return true;
    }

    bool AppearanceImpl::setDepthWrite(EDepthWrite flag)
    {
        getIScene().setRenderStateDepthWrite(m_renderStateHandle, flag);
        return true;
    }

    bool AppearanceImpl::getDepthWriteMode(EDepthWrite& mode) const
    {
        mode = getIScene().getRenderState(m_renderStateHandle).depthWrite;
        return true;
    }

    bool AppearanceImpl::setScissorTest(EScissorTest flag, int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        getIScene().setRenderStateScissorTest(m_renderStateHandle, flag, { x, y, width, height });
        return true;
    }

    bool AppearanceImpl::getScissorTestState(EScissorTest& mode) const
    {
        mode = getIScene().getRenderState(m_renderStateHandle).scissorTest;
        return true;
    }

    bool AppearanceImpl::getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const
    {
        const auto& scissorRegion = getIScene().getRenderState(m_renderStateHandle).scissorRegion;
        x = scissorRegion.x;
        y = scissorRegion.y;
        width = scissorRegion.width;
        height = scissorRegion.height;

        return true;
    }

    bool AppearanceImpl::setStencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        getIScene().setRenderStateStencilFunc(m_renderStateHandle, func, ref, mask);
        return true;
    }

    bool AppearanceImpl::getStencilFunc(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const
    {
        const RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        func = rs.stencilFunc;
        ref  = rs.stencilRefValue;
        mask = rs.stencilMask;
        return true;
    }

    bool AppearanceImpl::setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass)
    {
        getIScene().setRenderStateStencilOps( m_renderStateHandle, sfail, dpfail, dppass);
        return true;
    }

    bool AppearanceImpl::getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const
    {
        const RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        sfail  = rs.stencilOpFail;
        dpfail = rs.stencilOpDepthFail;
        dppass = rs.stencilOpDepthPass;
        return true;
    }

    bool AppearanceImpl::setCullingMode(ECullMode mode)
    {
        getIScene().setRenderStateCullMode(m_renderStateHandle, mode);
        return true;
    }

    bool AppearanceImpl::getCullingMode(ECullMode& mode) const
    {
        mode = getIScene().getRenderState(m_renderStateHandle).cullMode;
        return true;
    }

    bool AppearanceImpl::setDrawMode(EDrawMode mode)
    {
        if (m_effectImpl->hasGeometryShader())
        {
            EDrawMode geometryShaderInputType;
            m_effectImpl->getGeometryShaderInputType(geometryShaderInputType);
            if (!AppearanceUtils::GeometryShaderCompatibleWithDrawMode(geometryShaderInputType, mode))
            {
                getErrorReporting().set("Appearance::setDrawMode failed, source Effect has a geometry shader which expects a different draw mode.");
                return false;
            }
        }

        getIScene().setRenderStateDrawMode(m_renderStateHandle, mode);
        return true;
    }

    bool AppearanceImpl::getDrawMode(EDrawMode& mode) const
    {
        mode = getIScene().getRenderState(m_renderStateHandle).drawMode;
        return true;
    }

    bool AppearanceImpl::setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
    {
        const ColorWriteMask colorMask =
            (writeRed   ? static_cast<ColorWriteMask>(EColorWriteFlag_Red  ) : 0u) |
            (writeGreen ? static_cast<ColorWriteMask>(EColorWriteFlag_Green) : 0u) |
            (writeBlue  ? static_cast<ColorWriteMask>(EColorWriteFlag_Blue ) : 0u) |
            (writeAlpha ? static_cast<ColorWriteMask>(EColorWriteFlag_Alpha) : 0u);
        getIScene().setRenderStateColorWriteMask(m_renderStateHandle, colorMask);
        return true;
    }

    bool AppearanceImpl::getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const
    {
        const ColorWriteMask mask = getIScene().getRenderState(m_renderStateHandle).colorWriteMask;
        writeRed   = (mask & EColorWriteFlag_Red  ) != 0;
        writeGreen = (mask & EColorWriteFlag_Green) != 0;
        writeBlue  = (mask & EColorWriteFlag_Blue ) != 0;
        writeAlpha = (mask & EColorWriteFlag_Alpha) != 0;
        return true;
    }

    bool AppearanceImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << (m_effectImpl ? serializationContext.getIDForObject(m_effectImpl) : SerializationContext::GetObjectIDNull());

        outStream << m_renderStateHandle;
        outStream << m_uniformLayout;
        outStream << m_uniformInstance;

        outStream << static_cast<uint32_t>(m_bindableInputs.size());
        for (const auto& bindableInput : m_bindableInputs)
        {
            outStream << bindableInput.key;
            outStream << (bindableInput.value.externallyBoundDataObject ? serializationContext.getIDForObject(bindableInput.value.externallyBoundDataObject) : SerializationContext::GetObjectIDNull());
            outStream << bindableInput.value.dataReference;
        }

        return true;
    }

    bool AppearanceImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_effectImpl);

        inStream >> m_renderStateHandle;
        inStream >> m_uniformLayout;
        inStream >> m_uniformInstance;

        uint32_t bindableInputCount = 0u;
        inStream >> bindableInputCount;

        assert(m_bindableInputs.size() == 0u);
        m_bindableInputs.reserve(bindableInputCount);
        for (uint32_t i = 0u; i < bindableInputCount; ++i)
        {
            uint32_t inputIndex = 0u;
            inStream >> inputIndex;

            BindableInput bindableInput;
            DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, bindableInput.externallyBoundDataObject);
            inStream >> bindableInput.dataReference;

            m_bindableInputs.put(inputIndex, bindableInput);
        }

        serializationContext.addForDependencyResolve(this);

        return true;
    }

    bool AppearanceImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_effectImpl);

        for (auto& bindableInput : m_bindableInputs)
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(bindableInput.value.externallyBoundDataObject);

        return true;
    }

    void AppearanceImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);
        validateEffect(report);
        validateUniforms(report);
    }

    void AppearanceImpl::validateEffect(ValidationReportImpl& report) const
    {
        ObjectIteratorImpl iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::Effect);
        RamsesObject* ramsesObject = iter.getNext();
        while (nullptr != ramsesObject)
        {
            const Effect& effect = RamsesObjectTypeUtils::ConvertTo<Effect>(*ramsesObject);
            if (&effect.impl() == m_effectImpl)
            {
                report.addDependentObject(*this, effect.impl());
                return;
            }

            ramsesObject = iter.getNext();
        }

        report.add(EIssueType::Error, "Appearance is referring to an invalid Effect", &getRamsesObject());
    }

    void AppearanceImpl::validateUniforms(ValidationReportImpl& report) const
    {
        const DataLayout& layout = getIScene().getDataLayout(m_uniformLayout);
        const uint32_t numFields = layout.getFieldCount();
        for (DataFieldHandle fieldHandle(0u); fieldHandle < numFields; ++fieldHandle)
        {
            const EDataType dataType = layout.getField(fieldHandle).dataType;
            if (!IsTextureSamplerType(dataType))
                continue;

            const TextureSamplerHandle samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, fieldHandle);
            if (!getIScene().isTextureSamplerAllocated(samplerHandle))
                report.add(EIssueType::Error, "Appearance is using a Texture Sampler that does not exist", &getRamsesObject());

            SceneObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::TextureSampler);
            while (const auto* sampler = iter.getNext<ramses::TextureSampler>())
            {
                if (samplerHandle == sampler->impl().getTextureSamplerHandle())
                {
                    report.addDependentObject(*this, sampler->impl());
                    break;
                }
            }
        }

        for (const auto& bindableInput : m_bindableInputs)
        {
            if (bindableInput.value.externallyBoundDataObject)
            {
                const DataInstanceHandle boundInstance = getIScene().getDataReference(m_uniformInstance, DataFieldHandle(bindableInput.key));
                if (!getIScene().isDataInstanceAllocated(boundInstance))
                    report.add(EIssueType::Error, "Appearance's input is bound to a DataObject that does not exist", &getRamsesObject());

                ObjectIteratorImpl iterator(getSceneImpl().getObjectRegistry(), ERamsesObjectType::DataObject);
                RamsesObject* ramsesObject = nullptr;
                while (nullptr != (ramsesObject = iterator.getNext()))
                {
                    const DataObject& dataObject = RamsesObjectTypeUtils::ConvertTo<DataObject>(*ramsesObject);
                    if (boundInstance == dataObject.impl().getDataReference())
                    {
                        report.addDependentObject(*this, dataObject.impl());
                        break;
                    }
                }
            }
        }
    }

    void AppearanceImpl::initializeFrameworkData(const EffectImpl& effect)
    {
        m_effectImpl = &effect;

        m_renderStateHandle = getIScene().allocateRenderState(RenderStateHandle::Invalid());
        createUniformDataInstance(effect);

        // Set draw mode to geometry shader's expected mode, if effect has such
        if (effect.hasGeometryShader())
        {
            EDrawMode geometryShaderInputType;
            effect.getGeometryShaderInputType(geometryShaderInputType);
            setDrawMode(geometryShaderInputType);
        }
    }

    void AppearanceImpl::deinitializeFrameworkData()
    {
        getIScene().releaseDataInstance(m_uniformInstance);
        m_uniformInstance = DataInstanceHandle::Invalid();

        for(const auto& bindableInput : m_bindableInputs)
        {
            const DataInstanceHandle dataRef = bindableInput.value.dataReference;
            const DataLayoutHandle dataRefLayout = getIScene().getLayoutOfDataInstance(dataRef);
            getIScene().releaseDataInstance(dataRef);
            getIScene().releaseDataLayout(dataRefLayout);
        }
        m_bindableInputs.clear();

        getIScene().releaseRenderState(m_renderStateHandle);
        m_renderStateHandle = RenderStateHandle::Invalid();

        getIScene().releaseDataLayout(m_uniformLayout);
        m_uniformLayout = DataLayoutHandle::Invalid();
    }

    const EffectImpl* AppearanceImpl::getEffectImpl() const
    {
        return m_effectImpl;
    }

    const Effect& AppearanceImpl::getEffect() const
    {
        return RamsesObjectTypeUtils::ConvertTo<Effect>(m_effectImpl->getRamsesObject());
    }

    RenderStateHandle AppearanceImpl::getRenderStateHandle() const
    {
        return m_renderStateHandle;
    }

    DataInstanceHandle AppearanceImpl::getUniformDataInstance() const
    {
        return m_uniformInstance;
    }

    void AppearanceImpl::createUniformDataInstance(const EffectImpl& effect)
    {
        InputIndexVector referencedInputs;
        const EffectInputInformationVector& uniformsInputInfo = effect.getUniformInputInformation();
        m_uniformLayout = DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(getIScene(), uniformsInputInfo, referencedInputs, effect.getLowlevelResourceHash());

        m_uniformInstance = getIScene().allocateDataInstance(m_uniformLayout, {});

        m_bindableInputs.reserve(m_bindableInputs.size() + referencedInputs.size());
        for (const auto& refInput : referencedInputs)
        {
            const DataFieldHandle dataField(refInput);

            BindableInput bindableInput;
            bindableInput.externallyBoundDataObject = nullptr;
            bindableInput.dataReference = DataLayoutCreationHelper::CreateAndBindDataReference(getIScene(), m_uniformInstance, dataField, uniformsInputInfo[refInput].dataType);
            m_bindableInputs.put(refInput, bindableInput);
        }
    }

    bool AppearanceImpl::checkEffectInputValidityAndValueCompatibility(const EffectInputImpl& input, size_t valueElementCount, std::initializer_list<ramses::internal::EDataType> valueDataType) const
    {
        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            getErrorReporting().set("Appearance::set failed, input is not properly initialized or cannot be used with this appearance.");
            return false;
        }

        const auto result = std::find(valueDataType.begin(), valueDataType.end(), input.getInternalDataType());
        if (result == valueDataType.end())
        {
            getErrorReporting().set(::fmt::format("Appearance::set failed, value type does not match input data type {}", EnumToString(input.getInternalDataType())));
            return false;
        }

        if (input.getElementCount() != valueElementCount)
        {
            getErrorReporting().set("Appearance::set failed, element count does not match");
            return false;
        }

        return true;
    }

    DataInstanceHandle AppearanceImpl::getDataReference(DataFieldHandle dataField, [[maybe_unused]] ramses::internal::EDataType expectedDataType) const
    {
        const DataInstanceHandle dataReference = getIScene().getDataReference(m_uniformInstance, dataField);
        assert(getIScene().getDataLayout(m_uniformLayout).getField(dataField).elementCount == 1u);
        assert(getIScene().getDataLayout(getIScene().getLayoutOfDataInstance(dataReference)).getField(DataFieldHandle(0u)).dataType == expectedDataType);

        return dataReference;
    }

    template <typename T>
    bool AppearanceImpl::setInputValue(const EffectInputImpl& input, size_t elementCount, const T* valuesIn)
    {
        return setDataArrayChecked(elementCount, valuesIn, input);
    }
    template <typename T>
    bool AppearanceImpl::getInputValue(const EffectInputImpl& input, size_t elementCount, T* valuesOut) const
    {
        return getDataArrayChecked(elementCount, valuesOut, input);
    }

    template <typename T>
    bool AppearanceImpl::setDataArrayChecked(size_t elementCount, const T* values, const EffectInputImpl& input)
    {
        if (input.getSemantics() != EFixedSemantics::Invalid)
        {
            getErrorReporting().set("Appearance::set failed, can't access value of semantic uniform");
            return false;
        }
        if (!checkEffectInputValidityAndValueCompatibility(input, elementCount, { TypeToEDataTypeTraits<T>::DataType }))
            return false;

        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        const BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        const bool isBindable = (bindableInput != nullptr);
        if (isBindable && bindableInput->externallyBoundDataObject)
        {
            getErrorReporting().set("Appearance::set failed, given uniform input is currently bound to a DataObject. Either unbind it from input first or set value on the DataObject itself.");
            return false;
        }

        const DataFieldHandle dataField(inputIndex);
        if (isBindable)
        {
            const DataInstanceHandle dataReference = getDataReference(dataField, input.getInternalDataType());
            const T* currentValues = ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), dataReference, DataFieldHandle(0u));
            if (PlatformMemory::Compare(currentValues, values, 1u * sizeof(T)) != 0)
            {
                ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), dataReference, DataFieldHandle(0u), 1u, values);
            }
        }
        else
        {
            static_assert( std::is_same_v<decltype(DataFieldInfo::elementCount), uint32_t> == true );
            assert(getIScene().getDataLayout(m_uniformLayout).getField(dataField).elementCount == static_cast<uint32_t>(elementCount));
            const T* currentValues = ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_uniformInstance, dataField);
            if (PlatformMemory::Compare(currentValues, values, elementCount * sizeof(T)) != 0)
            {
                ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), m_uniformInstance, dataField, static_cast<uint32_t>(elementCount), values);
            }
        }

        return true;
    }

    template <typename T>
    bool AppearanceImpl::getDataArrayChecked(size_t elementCount, T* values, const EffectInputImpl& input) const
    {
        if (input.getSemantics() != EFixedSemantics::Invalid)
        {
            getErrorReporting().set("Appearance::set failed, can't access value of semantic uniform");
            return false;
        }
        if (!checkEffectInputValidityAndValueCompatibility(input, elementCount, { TypeToEDataTypeTraits<T>::DataType }))
            return false;

        const BindableInput* bindableInput = m_bindableInputs.get(static_cast<uint32_t>(input.getInputIndex()));
        const bool isBindable = (bindableInput != nullptr);
        if (isBindable && bindableInput->externallyBoundDataObject)
        {
            getErrorReporting().set("Appearance::get failed, given uniform input is currently bound to a DataObject. Either unbind it from input first or get value from the DataObject itself.");
            return false;
        }

        const DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()));
        if (isBindable)
        {
            const DataInstanceHandle dataReference = getDataReference(dataField, input.getInternalDataType());
            PlatformMemory::Copy(values, ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), dataReference, DataFieldHandle(0u)), EnumToSize(input.getInternalDataType()));
        }
        else
        {
            PlatformMemory::Copy(values, ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_uniformInstance, dataField), elementCount * EnumToSize(input.getInternalDataType()));
        }

        return true;
    }

    bool AppearanceImpl::setInputTexture(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler)
    {
        if (!isFromTheSameSceneAs(textureSampler))
        {
            getErrorReporting().set("Appearance::setInputTexture failed, textureSampler is not from the same scene as this appearance");
            return false;
        }

        return setInputTextureInternal(input, textureSampler);
    }

    bool AppearanceImpl::getInputTexture(const EffectInputImpl& input, const ramses::TextureSampler*& textureSampler)
    {
        textureSampler = nullptr;
        if (!checkEffectInputValidityAndValueCompatibility(input, 1u,
            {ramses::internal::EDataType::TextureSampler2D, ramses::internal::EDataType::TextureSampler3D, ramses::internal::EDataType::TextureSamplerCube}))
            return false;

        const DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()));
        const auto samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, dataField);
        if (samplerHandle.isValid())
        {
            SceneObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::TextureSampler);
            while (const auto* sampler = iter.getNext<ramses::TextureSampler>())
            {
                if (samplerHandle == sampler->impl().getTextureSamplerHandle())
                {
                    textureSampler = sampler;
                    break;
                }
            }
        }
        return true;
    }

    bool AppearanceImpl::getInputTextureMS(const EffectInputImpl& input, const TextureSamplerMS*& textureSampler)
    {
        textureSampler = nullptr;
        if (!checkEffectInputValidityAndValueCompatibility(input, 1u,
            {ramses::internal::EDataType::TextureSampler2DMS}))
            return false;

        const DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()));
        const auto samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, dataField);
        if (samplerHandle.isValid())
        {
            SceneObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::TextureSamplerMS);
            while (const auto* sampler = iter.getNext<TextureSamplerMS>())
            {
                if (samplerHandle == sampler->impl().getTextureSamplerHandle())
                {
                    textureSampler = sampler;
                    break;
                }
            }
        }
        return true;
    }

    bool AppearanceImpl::getInputTextureExternal(const EffectInputImpl& input, const TextureSamplerExternal*& textureSampler)
    {
        textureSampler = nullptr;
        if (!checkEffectInputValidityAndValueCompatibility(input, 1u,
            {ramses::internal::EDataType::TextureSamplerExternal}))
            return false;

        const DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()));
        const auto samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, dataField);
        if (samplerHandle.isValid())
        {
            SceneObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::TextureSamplerExternal);
            while (const auto* sampler = iter.getNext<TextureSamplerExternal>())
            {
                if (samplerHandle == sampler->impl().getTextureSamplerHandle())
                {
                    textureSampler = sampler;
                    break;
                }
            }
        }
        return true;
    }

    bool AppearanceImpl::bindInput(const EffectInputImpl& input, const DataObjectImpl& dataObject)
    {
        if (!isFromTheSameSceneAs(dataObject))
        {
            getErrorReporting().set("Appearance::bindInput failed, dataObject is not from the same scene as this appearance");
            return false;
        }

        if (!checkEffectInputValidityAndValueCompatibility(input, 1u, {DataTypeUtils::ConvertDataTypeToInternal(dataObject.getDataType())}))
            return false;

        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput == nullptr)
        {
            getErrorReporting().set("Appearance::bindInput failed, given uniform input cannot be bound to a DataObject.");
            return false;
        }

        return bindInputInternal(input, dataObject);
    }

    bool AppearanceImpl::unbindInput(const EffectInputImpl& input)
    {
        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput == nullptr || !bindableInput->externallyBoundDataObject)
        {
            getErrorReporting().set("Appearance::unbindInput failed, given uniform input is not bound to a DataObject.");
            return false;
        }

        return unbindInputInternal(input);
    }

    bool AppearanceImpl::isInputBound(const EffectInputImpl& input) const
    {
        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        const BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        return (bindableInput != nullptr) && bindableInput->externallyBoundDataObject != nullptr;
    }

    bool AppearanceImpl::setInputTextureInternal(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler)
    {
        if (!checkEffectInputValidityAndValueCompatibility(input, 1u, {textureSampler.getTextureDataType()}))
            return false;

        const DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()));
        const TextureSamplerHandle samplerHandle = textureSampler.getTextureSamplerHandle();
        getIScene().setDataTextureSamplerHandle(m_uniformInstance, dataField, samplerHandle);
        return true;
    }

    bool AppearanceImpl::bindInputInternal(const EffectInputImpl& input, const DataObjectImpl& dataObject)
    {
        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        const DataFieldHandle dataField(inputIndex);
        getIScene().setDataReference(m_uniformInstance, dataField, dataObject.getDataReference());

        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        assert(bindableInput != nullptr);
        bindableInput->externallyBoundDataObject = &dataObject;

        return true;
    }

    bool AppearanceImpl::unbindInputInternal(const EffectInputImpl& input)
    {
        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        const DataFieldHandle dataField(inputIndex);
        getIScene().setDataReference(m_uniformInstance, dataField, bindableInput->dataReference);
        bindableInput->externallyBoundDataObject = nullptr;

        return true;
    }

    DataLayoutHandle AppearanceImpl::getUniformDataLayout() const
    {
        return m_uniformLayout;
    }

    const DataObject* AppearanceImpl::getBoundDataObject(const EffectInputImpl& input) const
    {
        const auto inputIndex = static_cast<uint32_t>(input.getInputIndex());
        const BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput && bindableInput->externallyBoundDataObject)
            return &RamsesObjectTypeUtils::ConvertTo<DataObject>(bindableInput->externallyBoundDataObject->getRamsesObject());

        return nullptr;
    }

    template bool AppearanceImpl::setInputValue<bool>(const EffectInputImpl&, size_t, const bool*);
    template bool AppearanceImpl::getInputValue<bool>(const EffectInputImpl&, size_t, bool*) const;
    template bool AppearanceImpl::setInputValue<int32_t>(const EffectInputImpl&, size_t, const int32_t*);
    template bool AppearanceImpl::getInputValue<int32_t>(const EffectInputImpl&, size_t, int32_t*) const;
    template bool AppearanceImpl::setInputValue<float>(const EffectInputImpl&, size_t, const float*);
    template bool AppearanceImpl::getInputValue<float>(const EffectInputImpl&, size_t, float*) const;
    template bool AppearanceImpl::setInputValue<vec2i>(const EffectInputImpl&, size_t, const vec2i*);
    template bool AppearanceImpl::getInputValue<vec2i>(const EffectInputImpl&, size_t, vec2i*) const;
    template bool AppearanceImpl::setInputValue<vec3i>(const EffectInputImpl&, size_t, const vec3i*);
    template bool AppearanceImpl::getInputValue<vec3i>(const EffectInputImpl&, size_t, vec3i*) const;
    template bool AppearanceImpl::setInputValue<vec4i>(const EffectInputImpl&, size_t, const vec4i*);
    template bool AppearanceImpl::getInputValue<vec4i>(const EffectInputImpl&, size_t, vec4i*) const;
    template bool AppearanceImpl::setInputValue<vec2f>(const EffectInputImpl&, size_t, const vec2f*);
    template bool AppearanceImpl::getInputValue<vec2f>(const EffectInputImpl&, size_t, vec2f*) const;
    template bool AppearanceImpl::setInputValue<vec3f>(const EffectInputImpl&, size_t, const vec3f*);
    template bool AppearanceImpl::getInputValue<vec3f>(const EffectInputImpl&, size_t, vec3f*) const;
    template bool AppearanceImpl::setInputValue<vec4f>(const EffectInputImpl&, size_t, const vec4f*);
    template bool AppearanceImpl::getInputValue<vec4f>(const EffectInputImpl&, size_t, vec4f*) const;
    template bool AppearanceImpl::setInputValue<matrix22f>(const EffectInputImpl&, size_t, const matrix22f*);
    template bool AppearanceImpl::getInputValue<matrix22f>(const EffectInputImpl&, size_t, matrix22f*) const;
    template bool AppearanceImpl::setInputValue<matrix33f>(const EffectInputImpl&, size_t, const matrix33f*);
    template bool AppearanceImpl::getInputValue<matrix33f>(const EffectInputImpl&, size_t, matrix33f*) const;
    template bool AppearanceImpl::setInputValue<matrix44f>(const EffectInputImpl&, size_t, const matrix44f*);
    template bool AppearanceImpl::getInputValue<matrix44f>(const EffectInputImpl&, size_t, matrix44f*) const;
}
