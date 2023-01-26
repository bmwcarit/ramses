//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// client API
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/DataObject.h"

// internal
#include "AppearanceImpl.h"
#include "EffectImpl.h"
#include "EffectInputImpl.h"
#include "ObjectIteratorImpl.h"
#include "TextureSamplerImpl.h"
#include "DataObjectImpl.h"
#include "AppearanceUtils.h"
#include "SerializationContext.h"
#include "SceneImpl.h"
#include "RamsesObjectRegistryIterator.h"
#include "Scene/ClientScene.h"
#include "SceneUtils/DataLayoutCreationHelper.h"
#include "SceneUtils/ISceneDataArrayAccessor.h"
#include "SceneUtils/DataInstanceHelper.h"
#include "Math3d/Matrix22f.h"
#include "SceneAPI/EDataType.h"
#include "ObjectIteratorImpl.h"
#include <algorithm>

namespace ramses
{
    AppearanceImpl::AppearanceImpl(SceneImpl& scene, const char* appearancename)
        : SceneObjectImpl(scene, ERamsesObjectType_Appearance, appearancename)
        , m_effectImpl(nullptr)
    {
    }

    AppearanceImpl::~AppearanceImpl()
    {
    }

    status_t AppearanceImpl::setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        getIScene().setRenderStateBlendFactors(m_renderStateHandle,
            AppearanceUtils::GetBlendFactorInternal(srcColor),
            AppearanceUtils::GetBlendFactorInternal(destColor),
            AppearanceUtils::GetBlendFactorInternal(srcAlpha),
            AppearanceUtils::GetBlendFactorInternal(destAlpha));
        return StatusOK;
    }

    ramses::status_t AppearanceImpl::getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const
    {
        const ramses_internal::RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        srcColor  = AppearanceUtils::GetBlendFactorFromInternal(rs.blendFactorSrcColor);
        destColor = AppearanceUtils::GetBlendFactorFromInternal(rs.blendFactorDstColor);
        srcAlpha  = AppearanceUtils::GetBlendFactorFromInternal(rs.blendFactorSrcAlpha);
        destAlpha = AppearanceUtils::GetBlendFactorFromInternal(rs.blendFactorDstAlpha);
        return StatusOK;
    }

    status_t AppearanceImpl::setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        if ((operationColor == EBlendOperation_Disabled) != (operationAlpha == EBlendOperation_Disabled))
        {
            return addErrorEntry("Appearance::setBlendingOperations:  invalid combination - one of operationColor or operationAlpha disabled and the other not!");
        }

        getIScene().setRenderStateBlendOperations(m_renderStateHandle,
            AppearanceUtils::GetBlendOperationInternal(operationColor),
            AppearanceUtils::GetBlendOperationInternal(operationAlpha));

        return StatusOK;
    }

    status_t AppearanceImpl::getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const
    {
        const ramses_internal::RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        operationColor = AppearanceUtils::GetBlendOperationFromInternal(rs.blendOperationColor);
        operationAlpha = AppearanceUtils::GetBlendOperationFromInternal(rs.blendOperationAlpha);
        return StatusOK;
    }

    ramses::status_t AppearanceImpl::setBlendingColor(float red, float green, float blue, float alpha)
    {
        getIScene().setRenderStateBlendColor(m_renderStateHandle, { red, green, blue, alpha });
        return StatusOK;
    }

    ramses::status_t AppearanceImpl::getBlendingColor(float& red, float& green, float& blue, float& alpha) const
    {
        const ramses_internal::RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        red = rs.blendColor.r;
        green = rs.blendColor.g;
        blue = rs.blendColor.b;
        alpha = rs.blendColor.a;

        return StatusOK;
    }

    status_t AppearanceImpl::setDepthFunction(EDepthFunc func)
    {
        getIScene().setRenderStateDepthFunc(m_renderStateHandle, AppearanceUtils::GetDepthFuncInternal(func));
        return StatusOK;
    }

    status_t AppearanceImpl::getDepthFunction(EDepthFunc& func) const
    {
        func = AppearanceUtils::GetDepthFuncFromInternal(getIScene().getRenderState(m_renderStateHandle).depthFunc);
        return StatusOK;
    }

    status_t AppearanceImpl::setDepthWrite(EDepthWrite flag)
    {
        getIScene().setRenderStateDepthWrite(m_renderStateHandle, AppearanceUtils::GetDepthWriteInternal(flag));
        return StatusOK;
    }

    ramses::status_t AppearanceImpl::getDepthWriteMode(EDepthWrite& mode) const
    {
        mode = AppearanceUtils::GetDepthWriteFromInternal(getIScene().getRenderState(m_renderStateHandle).depthWrite);
        return StatusOK;
    }

    status_t AppearanceImpl::setScissorTest(EScissorTest flag, int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        getIScene().setRenderStateScissorTest(m_renderStateHandle, AppearanceUtils::GetScissorTestInternal(flag), { x, y, width, height });
        return StatusOK;
    }

    status_t AppearanceImpl::getScissorTestState(EScissorTest& mode) const
    {
        mode = AppearanceUtils::GetScissorTestFromInternal(getIScene().getRenderState(m_renderStateHandle).scissorTest);
        return StatusOK;
    }

    status_t AppearanceImpl::getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const
    {
        const auto& scissorRegion = getIScene().getRenderState(m_renderStateHandle).scissorRegion;
        x = scissorRegion.x;
        y = scissorRegion.y;
        width = scissorRegion.width;
        height = scissorRegion.height;

        return StatusOK;
    }

    status_t AppearanceImpl::setStencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        getIScene().setRenderStateStencilFunc(m_renderStateHandle, AppearanceUtils::GetStencilFuncInternal(func), ref, mask);
        return StatusOK;
    }

    status_t AppearanceImpl::getStencilFunc(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const
    {
        const ramses_internal::RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        func = AppearanceUtils::GetStencilFuncFromInternal(rs.stencilFunc);
        ref  = rs.stencilRefValue;
        mask = rs.stencilMask;
        return StatusOK;
    }

    status_t AppearanceImpl::setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass)
    {
        getIScene().setRenderStateStencilOps(
            m_renderStateHandle,
            AppearanceUtils::GetStencilOperationInternal(sfail),
            AppearanceUtils::GetStencilOperationInternal(dpfail),
            AppearanceUtils::GetStencilOperationInternal(dppass));
        return StatusOK;
    }

    status_t AppearanceImpl::getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const
    {
        const ramses_internal::RenderState& rs = getIScene().getRenderState(m_renderStateHandle);
        sfail  = AppearanceUtils::GetStencilOperationFromInternal(rs.stencilOpFail);
        dpfail = AppearanceUtils::GetStencilOperationFromInternal(rs.stencilOpDepthFail);
        dppass = AppearanceUtils::GetStencilOperationFromInternal(rs.stencilOpDepthPass);
        return StatusOK;
    }

    status_t AppearanceImpl::setCullingMode(ECullMode mode)
    {
        getIScene().setRenderStateCullMode(m_renderStateHandle, AppearanceUtils::GetCullModeInternal(mode));
        return StatusOK;
    }

    status_t AppearanceImpl::getCullingMode(ECullMode& mode) const
    {
        mode = AppearanceUtils::GetCullModeFromInternal(getIScene().getRenderState(m_renderStateHandle).cullMode);
        return StatusOK;
    }

    status_t AppearanceImpl::setDrawMode(EDrawMode mode)
    {
        if (m_effectImpl->hasGeometryShader())
        {
            EDrawMode geometryShaderInputType;
            m_effectImpl->getGeometryShaderInputType(geometryShaderInputType);
            if (!AppearanceUtils::GeometryShaderCompatibleWithDrawMode(geometryShaderInputType, mode))
            {
                return addErrorEntry("Appearance::setDrawMode failed, source Effect has a geometry shader which expects a different draw mode.");
            }
        }

        getIScene().setRenderStateDrawMode(m_renderStateHandle, AppearanceUtils::GetDrawModeInternal(mode));
        return StatusOK;
    }

    status_t AppearanceImpl::getDrawMode(EDrawMode& mode) const
    {
        mode = AppearanceUtils::GetDrawModeFromInternal(getIScene().getRenderState(m_renderStateHandle).drawMode);
        return StatusOK;
    }

    status_t AppearanceImpl::setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
    {
        const ramses_internal::ColorWriteMask colorMask =
            (writeRed   ? static_cast<ramses_internal::ColorWriteMask>(ramses_internal::EColorWriteFlag_Red  ) : 0u) |
            (writeGreen ? static_cast<ramses_internal::ColorWriteMask>(ramses_internal::EColorWriteFlag_Green) : 0u) |
            (writeBlue  ? static_cast<ramses_internal::ColorWriteMask>(ramses_internal::EColorWriteFlag_Blue ) : 0u) |
            (writeAlpha ? static_cast<ramses_internal::ColorWriteMask>(ramses_internal::EColorWriteFlag_Alpha) : 0u);
        getIScene().setRenderStateColorWriteMask(m_renderStateHandle, colorMask);
        return StatusOK;
    }

    status_t AppearanceImpl::getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const
    {
        const ramses_internal::ColorWriteMask mask = getIScene().getRenderState(m_renderStateHandle).colorWriteMask;
        writeRed   = (mask & ramses_internal::EColorWriteFlag_Red  ) != 0;
        writeGreen = (mask & ramses_internal::EColorWriteFlag_Green) != 0;
        writeBlue  = (mask & ramses_internal::EColorWriteFlag_Blue ) != 0;
        writeAlpha = (mask & ramses_internal::EColorWriteFlag_Alpha) != 0;
        return StatusOK;
    }

    status_t AppearanceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << (m_effectImpl ? serializationContext.getIDForObject(m_effectImpl) : serializationContext.GetObjectIDNull());

        outStream << m_renderStateHandle;
        outStream << m_uniformLayout;
        outStream << m_uniformInstance;

        outStream << static_cast<uint32_t>(m_bindableInputs.size());
        for (const auto& bindableInput : m_bindableInputs)
        {
            outStream << bindableInput.key;
            outStream << (bindableInput.value.externallyBoundDataObject ? serializationContext.getIDForObject(bindableInput.value.externallyBoundDataObject) : serializationContext.GetObjectIDNull());
            outStream << bindableInput.value.dataReference;
        }

        return StatusOK;
    }

    status_t AppearanceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_effectImpl);

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
            serializationContext.ReadDependentPointerAndStoreAsID(inStream, bindableInput.externallyBoundDataObject);
            inStream >> bindableInput.dataReference;

            m_bindableInputs.put(inputIndex, bindableInput);
        }

        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t AppearanceImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_effectImpl);

        for (auto& bindableInput : m_bindableInputs)
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(bindableInput.value.externallyBoundDataObject);

        return StatusOK;
    }

    status_t AppearanceImpl::validate() const
    {
        const auto status = SceneObjectImpl::validate();
        return std::max(status, std::max(validateEffect(), validateUniforms()));
    }

    status_t AppearanceImpl::validateEffect() const
    {
        ObjectIteratorImpl iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_Effect);
        RamsesObject* ramsesObject = iter.getNext();
        while (nullptr != ramsesObject)
        {
            const Effect& effect = RamsesObjectTypeUtils::ConvertTo<Effect>(*ramsesObject);
            if (&effect.impl == m_effectImpl)
                return addValidationOfDependentObject(*m_effectImpl);

            ramsesObject = iter.getNext();
        }

        return addValidationMessage(EValidationSeverity_Error, "Appearance is referring to an invalid Effect");
    }

    status_t AppearanceImpl::validateUniforms() const
    {
        status_t status = StatusOK;
        const ramses_internal::DataLayout& layout = getIScene().getDataLayout(m_uniformLayout);
        const uint32_t numFields = layout.getFieldCount();
        for (ramses_internal::DataFieldHandle fieldHandle(0u); fieldHandle < numFields; ++fieldHandle)
        {
            const ramses_internal::EDataType dataType = layout.getField(fieldHandle).dataType;
            if (!IsTextureSamplerType(dataType))
                continue;

            const ramses_internal::TextureSamplerHandle samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, fieldHandle);
            if (!getIScene().isTextureSamplerAllocated(samplerHandle))
                return addValidationMessage(EValidationSeverity_Error, "Appearance is using a Texture Sampler that does not exist");

            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_TextureSampler);
            while (const TextureSampler* sampler = iter.getNext<TextureSampler>())
            {
                if (samplerHandle == sampler->impl.getTextureSamplerHandle())
                {
                    status = std::max(status, addValidationOfDependentObject(sampler->impl));
                    break;
                }
            }
        }

        for (const auto& bindableInput : m_bindableInputs)
        {
            if (bindableInput.value.externallyBoundDataObject)
            {
                const ramses_internal::DataInstanceHandle boundInstance = getIScene().getDataReference(m_uniformInstance, ramses_internal::DataFieldHandle(bindableInput.key));
                if (!getIScene().isDataInstanceAllocated(boundInstance))
                    return addValidationMessage(EValidationSeverity_Error, "Appearance's input is bound to a DataObject that does not exist");

                ObjectIteratorImpl iterator(getSceneImpl().getObjectRegistry(), ERamsesObjectType_DataObject);
                RamsesObject* ramsesObject = nullptr;
                while (nullptr != (ramsesObject = iterator.getNext()))
                {
                    const DataObject& dataObject = RamsesObjectTypeUtils::ConvertTo<DataObject>(*ramsesObject);
                    if (boundInstance == dataObject.impl.getDataReference())
                    {
                        status = std::max(status, addValidationOfDependentObject(dataObject.impl));
                        break;
                    }
                }
            }
        }

        return status;
    }

    void AppearanceImpl::initializeFrameworkData(const EffectImpl& effect)
    {
        m_effectImpl = &effect;

        m_renderStateHandle = getIScene().allocateRenderState(ramses_internal::RenderStateHandle::Invalid());
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
        m_uniformInstance = ramses_internal::DataInstanceHandle::Invalid();

        for(const auto& bindableInput : m_bindableInputs)
        {
            const ramses_internal::DataInstanceHandle dataRef = bindableInput.value.dataReference;
            const ramses_internal::DataLayoutHandle dataRefLayout = getIScene().getLayoutOfDataInstance(dataRef);
            getIScene().releaseDataInstance(dataRef);
            getIScene().releaseDataLayout(dataRefLayout);
        }
        m_bindableInputs.clear();

        getIScene().releaseRenderState(m_renderStateHandle);
        m_renderStateHandle = ramses_internal::RenderStateHandle::Invalid();

        getIScene().releaseDataLayout(m_uniformLayout);
        m_uniformLayout = ramses_internal::DataLayoutHandle::Invalid();
    }

    const EffectImpl* AppearanceImpl::getEffectImpl() const
    {
        return m_effectImpl;
    }

    const Effect& AppearanceImpl::getEffect() const
    {
        return RamsesObjectTypeUtils::ConvertTo<Effect>(m_effectImpl->getRamsesObject());
    }

    ramses_internal::RenderStateHandle AppearanceImpl::getRenderStateHandle() const
    {
        return m_renderStateHandle;
    }

    ramses_internal::DataInstanceHandle AppearanceImpl::getUniformDataInstance() const
    {
        return m_uniformInstance;
    }

    void AppearanceImpl::createUniformDataInstance(const EffectImpl& effect)
    {
        ramses_internal::InputIndexVector referencedInputs;
        const ramses_internal::EffectInputInformationVector& uniformsInputInfo = effect.getUniformInputInformation();
        m_uniformLayout = ramses_internal::DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(getIScene(), uniformsInputInfo, referencedInputs, effect.getLowlevelResourceHash());

        m_uniformInstance = getIScene().allocateDataInstance(m_uniformLayout);

        m_bindableInputs.reserve(m_bindableInputs.size() + referencedInputs.size());
        for (const auto& refInput : referencedInputs)
        {
            const ramses_internal::DataFieldHandle dataField(refInput);

            BindableInput bindableInput;
            bindableInput.externallyBoundDataObject = nullptr;
            bindableInput.dataReference = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(getIScene(), m_uniformInstance, dataField, uniformsInputInfo[refInput].dataType);
            m_bindableInputs.put(refInput, bindableInput);
        }
    }

    status_t AppearanceImpl::checkEffectInputValidityAndValueCompatibility(const EffectInputImpl& input, uint32_t valueElementCount, std::initializer_list<ramses_internal::EDataType> valueDataType) const
    {
        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            return addErrorEntry("Appearance::set failed, input is not properly initialized or cannot be used with this appearance.");
        }

        const auto result = std::find(valueDataType.begin(), valueDataType.end(),input.getDataType());
        if (result == valueDataType.end())
        {
            return addErrorEntry(::fmt::format("Appearance::set failed, value type does not match input data type {}", EnumToString(input.getDataType())));
        }

        if (input.getElementCount() != valueElementCount)
        {
            return addErrorEntry("Appearance::set failed, element count does not match");
        }
        return StatusOK;
    }

    ramses_internal::DataInstanceHandle AppearanceImpl::getDataReference(ramses_internal::DataFieldHandle dataField, ramses_internal::EDataType expectedDataType) const
    {
        const ramses_internal::DataInstanceHandle dataReference = getIScene().getDataReference(m_uniformInstance, dataField);
        assert(getIScene().getDataLayout(m_uniformLayout).getField(dataField).elementCount == 1u);
        assert(getIScene().getDataLayout(getIScene().getLayoutOfDataInstance(dataReference)).getField(ramses_internal::DataFieldHandle(0u)).dataType == expectedDataType);
        UNUSED(expectedDataType);

        return dataReference;
    }

    template <typename T>
    status_t AppearanceImpl::setInputValue(const EffectInputImpl& input, uint32_t elementCount, const T* valuesIn)
    {
        return setDataArrayChecked(elementCount, valuesIn, input);
    }
    template <typename T>
    status_t AppearanceImpl::getInputValue(const EffectInputImpl& input, uint32_t elementCount, T* valuesOut) const
    {
        return getDataArrayChecked(elementCount, valuesOut, input);
    }

    template <typename ContainerT, typename ElementT>
    status_t AppearanceImpl::setInputValueWithElementTypeCast(const EffectInputImpl& input, uint32_t elementCount, const ElementT* valuesIn)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) TODO(tobias) questionable if correct because ContainerT not POD
        return setDataArrayChecked<ContainerT>(elementCount, reinterpret_cast<const ContainerT*>(valuesIn), input);
    }

    template <typename ContainerT, typename ElementT>
    status_t AppearanceImpl::getInputValueWithElementTypeCast(const EffectInputImpl& input, uint32_t elementCount, ElementT* valuesOut) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) TODO(tobias) questionable if correct because ContainerT not POD
        return getDataArrayChecked<ContainerT>(elementCount, reinterpret_cast<ContainerT*>(valuesOut), input);
    }

    template <typename T>
    status_t AppearanceImpl::setDataArrayChecked(uint32_t elementCount, const T* values, const EffectInputImpl& input)
    {
        if (input.getSemantics() != ramses_internal::EFixedSemantics::Invalid)
        {
            return addErrorEntry("Appearance::set failed, can't access value of semantic uniform");
        }
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, elementCount, {ramses_internal::TypeToEDataTypeTraits<T>::DataType}));

        const BindableInput* bindableInput = m_bindableInputs.get(input.getInputIndex());
        const bool isBindable = (bindableInput != nullptr);
        if (isBindable && bindableInput->externallyBoundDataObject)
        {
            return addErrorEntry("Appearance::set failed, given uniform input is currently bound to a DataObject. Either unbind it from input first or set value on the DataObject itself.");
        }

        const ramses_internal::DataFieldHandle dataField(input.getInputIndex());
        if (isBindable)
        {
            const ramses_internal::DataInstanceHandle dataReference = getDataReference(dataField, input.getDataType());
            const T* currentValues = ramses_internal::ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), dataReference, ramses_internal::DataFieldHandle(0u));
            if (ramses_internal::PlatformMemory::Compare(currentValues, values, 1u * sizeof(T)) != 0)
            {
                ramses_internal::ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), dataReference, ramses_internal::DataFieldHandle(0u), 1u, values);
            }
        }
        else
        {
            assert(getIScene().getDataLayout(m_uniformLayout).getField(dataField).elementCount == elementCount);
            const T* currentValues = ramses_internal::ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_uniformInstance, dataField);
            if (ramses_internal::PlatformMemory::Compare(currentValues, values, elementCount * sizeof(T)) != 0)
            {
                ramses_internal::ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), m_uniformInstance, dataField, elementCount, values);
            }
        }

        return StatusOK;
    }

    template <typename T>
    status_t AppearanceImpl::getDataArrayChecked(uint32_t elementCount, T* values, const EffectInputImpl& input) const
    {
        if (input.getSemantics() != ramses_internal::EFixedSemantics::Invalid)
        {
            return addErrorEntry("Appearance::set failed, can't access value of semantic uniform");
        }
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, elementCount, {ramses_internal::TypeToEDataTypeTraits<T>::DataType}));

        const BindableInput* bindableInput = m_bindableInputs.get(input.getInputIndex());
        const bool isBindable = (bindableInput != nullptr);
        if (isBindable && bindableInput->externallyBoundDataObject)
        {
            return addErrorEntry("Appearance::get failed, given uniform input is currently bound to a DataObject. Either unbind it from input first or get value from the DataObject itself.");
        }

        const ramses_internal::DataFieldHandle dataField(input.getInputIndex());
        if (isBindable)
        {
            const ramses_internal::DataInstanceHandle dataReference = getDataReference(dataField, input.getDataType());
            ramses_internal::PlatformMemory::Copy(values, ramses_internal::ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), dataReference, ramses_internal::DataFieldHandle(0u)), EnumToSize(input.getDataType()));
        }
        else
        {
            ramses_internal::PlatformMemory::Copy(values, ramses_internal::ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_uniformInstance, dataField), elementCount * EnumToSize(input.getDataType()));
        }

        return StatusOK;
    }

    status_t AppearanceImpl::setInputTexture(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler)
    {
        if (!isFromTheSameSceneAs(textureSampler))
        {
            return addErrorEntry("Appearance::setInputTexture failed, textureSampler is not from the same scene as this appearance");
        }

        return setInputTextureInternal(input, textureSampler);
    }

    status_t AppearanceImpl::getInputTexture(const EffectInputImpl& input, const TextureSampler*& textureSampler)
    {
        textureSampler = nullptr;
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u,
            {ramses_internal::EDataType::TextureSampler2D, ramses_internal::EDataType::TextureSampler3D, ramses_internal::EDataType::TextureSamplerCube}));

        const ramses_internal::DataFieldHandle dataField(input.getInputIndex());
        const auto samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, dataField);
        if (samplerHandle.isValid())
        {
            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_TextureSampler);
            while (const TextureSampler* sampler = iter.getNext<TextureSampler>())
            {
                if (samplerHandle == sampler->impl.getTextureSamplerHandle())
                {
                    textureSampler = sampler;
                    break;
                }
            }
        }
        return StatusOK;
    }

    status_t AppearanceImpl::getInputTextureMS(const EffectInputImpl& input, const TextureSamplerMS*& textureSampler)
    {
        textureSampler = nullptr;
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u,
            {ramses_internal::EDataType::TextureSampler2DMS}));

        const ramses_internal::DataFieldHandle dataField(input.getInputIndex());
        const auto samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, dataField);
        if (samplerHandle.isValid())
        {
            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_TextureSamplerMS);
            while (const TextureSamplerMS* sampler = iter.getNext<TextureSamplerMS>())
            {
                if (samplerHandle == sampler->impl.getTextureSamplerHandle())
                {
                    textureSampler = sampler;
                    break;
                }
            }
        }
        return StatusOK;
    }

    status_t AppearanceImpl::getInputTextureExternal(const EffectInputImpl& input, const TextureSamplerExternal*& textureSampler)
    {
        textureSampler = nullptr;
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u,
            {ramses_internal::EDataType::TextureSamplerExternal}));

        const ramses_internal::DataFieldHandle dataField(input.getInputIndex());
        const auto samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, dataField);
        if (samplerHandle.isValid())
        {
            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_TextureSamplerExternal);
            while (const TextureSamplerExternal* sampler = iter.getNext<TextureSamplerExternal>())
            {
                if (samplerHandle == sampler->impl.getTextureSamplerHandle())
                {
                    textureSampler = sampler;
                    break;
                }
            }
        }
        return StatusOK;
    }

    status_t AppearanceImpl::bindInput(const EffectInputImpl& input, const DataObjectImpl& dataObject)
    {
        if (!isFromTheSameSceneAs(dataObject))
        {
            return addErrorEntry("Appearance::bindInput failed, dataObject is not from the same scene as this appearance");
        }

        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u, {dataObject.getDataType()}));

        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput == nullptr)
        {
            return addErrorEntry("Appearance::bindInput failed, given uniform input cannot be bound to a DataObject.");
        }

        return bindInputInternal(input, dataObject);
    }

    status_t AppearanceImpl::unbindInput(const EffectInputImpl& input)
    {
        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput == nullptr || !bindableInput->externallyBoundDataObject)
        {
            return addErrorEntry("Appearance::unbindInput failed, given uniform input is not bound to a DataObject.");
        }

        return unbindInputInternal(input);
    }

    bool AppearanceImpl::isInputBound(const EffectInputImpl& input) const
    {
        const uint32_t inputIndex = input.getInputIndex();
        const BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        return (bindableInput != nullptr) && bindableInput->externallyBoundDataObject != nullptr;
    }

    status_t AppearanceImpl::setInputTextureInternal(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler)
    {
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u, {textureSampler.getTextureDataType()}));

        const ramses_internal::DataFieldHandle dataField(input.getInputIndex());
        const ramses_internal::TextureSamplerHandle samplerHandle = textureSampler.getTextureSamplerHandle();
        getIScene().setDataTextureSamplerHandle(m_uniformInstance, dataField, samplerHandle);
        return StatusOK;
    }

    status_t AppearanceImpl::bindInputInternal(const EffectInputImpl& input, const DataObjectImpl& dataObject)
    {
        const uint32_t inputIndex = input.getInputIndex();
        const ramses_internal::DataFieldHandle dataField(inputIndex);
        getIScene().setDataReference(m_uniformInstance, dataField, dataObject.getDataReference());

        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        assert(bindableInput != nullptr);
        bindableInput->externallyBoundDataObject = &dataObject;

        return StatusOK;
    }

    status_t AppearanceImpl::unbindInputInternal(const EffectInputImpl& input)
    {
        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        const ramses_internal::DataFieldHandle dataField(inputIndex);
        getIScene().setDataReference(m_uniformInstance, dataField, bindableInput->dataReference);
        bindableInput->externallyBoundDataObject = nullptr;

        return StatusOK;
    }

    ramses_internal::DataLayoutHandle AppearanceImpl::getUniformDataLayout() const
    {
        return m_uniformLayout;
    }

    const ramses::DataObject* AppearanceImpl::getBoundDataObject(const EffectInputImpl& input) const
    {
        const uint32_t inputIndex = input.getInputIndex();
        const BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput && bindableInput->externallyBoundDataObject)
            return &RamsesObjectTypeUtils::ConvertTo<DataObject>(bindableInput->externallyBoundDataObject->getRamsesObject());

        return nullptr;
    }

    template status_t AppearanceImpl::setInputValue<int32_t>(const EffectInputImpl&, uint32_t, const int32_t*);
    template status_t AppearanceImpl::getInputValue<int32_t>(const EffectInputImpl&, uint32_t, int32_t*) const;
    template status_t AppearanceImpl::setInputValue<float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValue<float>(const EffectInputImpl&, uint32_t, float*) const;
    template status_t AppearanceImpl::setInputValue<ramses_internal::Vector2i>(const EffectInputImpl&, uint32_t, const ramses_internal::Vector2i*);
    template status_t AppearanceImpl::getInputValue<ramses_internal::Vector2i>(const EffectInputImpl&, uint32_t, ramses_internal::Vector2i*) const;
    template status_t AppearanceImpl::setInputValue<ramses_internal::Vector3i>(const EffectInputImpl&, uint32_t, const ramses_internal::Vector3i*);
    template status_t AppearanceImpl::getInputValue<ramses_internal::Vector3i>(const EffectInputImpl&, uint32_t, ramses_internal::Vector3i*) const;
    template status_t AppearanceImpl::setInputValue<ramses_internal::Vector4i>(const EffectInputImpl&, uint32_t, const ramses_internal::Vector4i*);
    template status_t AppearanceImpl::getInputValue<ramses_internal::Vector4i>(const EffectInputImpl&, uint32_t, ramses_internal::Vector4i*) const;
    template status_t AppearanceImpl::setInputValue<ramses_internal::Vector2>(const EffectInputImpl&, uint32_t, const ramses_internal::Vector2*);
    template status_t AppearanceImpl::getInputValue<ramses_internal::Vector2>(const EffectInputImpl&, uint32_t, ramses_internal::Vector2*) const;
    template status_t AppearanceImpl::setInputValue<ramses_internal::Vector3>(const EffectInputImpl&, uint32_t, const ramses_internal::Vector3*);
    template status_t AppearanceImpl::getInputValue<ramses_internal::Vector3>(const EffectInputImpl&, uint32_t, ramses_internal::Vector3*) const;
    template status_t AppearanceImpl::setInputValue<ramses_internal::Vector4>(const EffectInputImpl&, uint32_t, const ramses_internal::Vector4*);
    template status_t AppearanceImpl::getInputValue<ramses_internal::Vector4>(const EffectInputImpl&, uint32_t, ramses_internal::Vector4*) const;

    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Vector2i, int32_t>(const EffectInputImpl&, uint32_t, const int32_t*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Vector2i, int32_t>(const EffectInputImpl&, uint32_t, int32_t*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Vector3i, int32_t>(const EffectInputImpl&, uint32_t, const int32_t*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Vector3i, int32_t>(const EffectInputImpl&, uint32_t, int32_t*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Vector4i, int32_t>(const EffectInputImpl&, uint32_t, const int32_t*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Vector4i, int32_t>(const EffectInputImpl&, uint32_t, int32_t*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Vector2, float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Vector2, float>(const EffectInputImpl&, uint32_t, float*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Vector3, float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Vector3, float>(const EffectInputImpl&, uint32_t, float*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Vector4, float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Vector4, float>(const EffectInputImpl&, uint32_t, float*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Matrix22f, float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Matrix22f, float>(const EffectInputImpl&, uint32_t, float*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Matrix33f, float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Matrix33f, float>(const EffectInputImpl&, uint32_t, float*) const;
    template status_t AppearanceImpl::setInputValueWithElementTypeCast<ramses_internal::Matrix44f, float>(const EffectInputImpl&, uint32_t, const float*);
    template status_t AppearanceImpl::getInputValueWithElementTypeCast<ramses_internal::Matrix44f, float>(const EffectInputImpl&, uint32_t, float*) const;
}
