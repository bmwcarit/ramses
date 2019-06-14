//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// client API
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/DataObject.h"

// internal
#include "AppearanceImpl.h"
#include "EffectImpl.h"
#include "EffectInputImpl.h"
#include "ObjectIteratorImpl.h"
#include "TextureSamplerImpl.h"
#include "ResourceIteratorImpl.h"
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

namespace ramses
{
    AppearanceImpl::AppearanceImpl(SceneImpl& scene, const char* appearancename)
        : SceneObjectImpl(scene, ERamsesObjectType_Appearance, appearancename)
        , m_effectImpl(NULL)
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
        CHECK_RETURN_ERR(serializeInternal(outStream, serializationContext));

        return StatusOK;
    }

    status_t AppearanceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(deserializeInternal(inStream, serializationContext));

        return StatusOK;
    }

    status_t AppearanceImpl::serializeInternal(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        resourceId_t resID = InvalidResourceId;
        if (0 != m_effectImpl)
        {
            resID = m_effectImpl->getResourceId();
        }
        outStream << resID.highPart;
        outStream << resID.lowPart;
        outStream << m_renderStateHandle;
        outStream << m_uniformLayout;
        outStream << m_uniformInstance;

        outStream << static_cast<uint32_t>(m_bindableInputs.count());
        for(const auto& bindableInput : m_bindableInputs)
        {
            outStream << bindableInput.key;
            outStream << bindableInput.value.externallyBound;
            outStream << bindableInput.value.dataReference;
        }

        return StatusOK;
    }

    status_t AppearanceImpl::deserializeInternal(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        resourceId_t resID;
        inStream >> resID.highPart;
        inStream >> resID.lowPart;
        if (resID != InvalidResourceId)
        {
            Resource* rImpl = getClientImpl().getHLResource_Threadsafe(resID);
            if (rImpl)
            {
                m_effectImpl = static_cast<EffectImpl*>(&rImpl->impl);
            }
            else
            {
                return addErrorEntry("AppearanceImpl::deserializeInternal failed, referenced effect resource is not available");
            }
        }

        inStream >> m_renderStateHandle;

        inStream >> m_uniformLayout;
        inStream >> m_uniformInstance;

        uint32_t bindableInputCount = 0u;
        inStream >> bindableInputCount;

        assert(m_bindableInputs.count() == 0u);
        m_bindableInputs.reserve(bindableInputCount);
        for (uint32_t i = 0u; i < bindableInputCount; ++i)
        {
            uint32_t inputIndex = 0u;
            inStream >> inputIndex;

            BindableInput bindableInput;
            inStream >> bindableInput.externallyBound;
            inStream >> bindableInput.dataReference;

            m_bindableInputs.put(inputIndex, bindableInput);
        }

        return StatusOK;
    }

    status_t AppearanceImpl::validate(uint32_t indent) const
    {
        status_t status = SceneObjectImpl::validate(indent);
        indent += IndentationStep;

        const status_t effectStatus = validateEffect(indent);
        if (StatusOK != effectStatus)
        {
            status = effectStatus;
        }

        const status_t uniformsStatus = validateUniforms(indent);
        if (StatusOK != uniformsStatus)
        {
            status = uniformsStatus;
        }

        return status;
    }

    status_t AppearanceImpl::validateEffect(uint32_t indent) const
    {
        ResourceIteratorImpl iter(getClientImpl(), ERamsesObjectType_Effect);
        RamsesObject* ramsesObject = iter.getNext();
        while (NULL != ramsesObject)
        {
            const Effect& effect = RamsesObjectTypeUtils::ConvertTo<Effect>(*ramsesObject);
            if (&effect.impl == m_effectImpl)
            {
                return addValidationOfDependentObject(indent, *m_effectImpl);
            }

            ramsesObject = iter.getNext();
        }

        addValidationMessage(EValidationSeverity_Error, indent, "Appearance is referring to an invalid Effect");
        return getValidationErrorStatus();
    }

    status_t AppearanceImpl::validateUniforms(uint32_t indent) const
    {
        status_t status = StatusOK;
        const ramses_internal::DataLayout& layout = getIScene().getDataLayout(m_uniformLayout);
        const uint32_t numFields = layout.getFieldCount();
        for (ramses_internal::DataFieldHandle fieldHandle(0u); fieldHandle < numFields; ++fieldHandle)
        {
            const ramses_internal::EDataType dataType = layout.getField(fieldHandle).dataType;
            if (dataType != ramses_internal::EDataType_TextureSampler)
            {
                continue;
            }

            const ramses_internal::TextureSamplerHandle samplerHandle = getIScene().getDataTextureSamplerHandle(m_uniformInstance, fieldHandle);
            if (!getIScene().isTextureSamplerAllocated(samplerHandle))
            {
                addValidationMessage(EValidationSeverity_Error, indent, "Appearance is using a Texture Sampler that does not exist");
                return getValidationErrorStatus();
            }

            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_TextureSampler);
            while (const TextureSampler* sampler = iter.getNext<TextureSampler>())
            {
                if (samplerHandle == sampler->impl.getTextureSamplerHandle())
                {
                    const status_t samplerStatus = addValidationOfDependentObject(indent, sampler->impl);
                    if (StatusOK != samplerStatus)
                    {
                        status = samplerStatus;
                    }
                    break;
                }
            }
        }

        for (const auto& bindableInput : m_bindableInputs)
        {
            if (bindableInput.value.externallyBound)
            {
                const ramses_internal::DataInstanceHandle boundInstance = getIScene().getDataReference(m_uniformInstance, ramses_internal::DataFieldHandle(bindableInput.key));
                if (!getIScene().isDataInstanceAllocated(boundInstance))
                {
                    addValidationMessage(EValidationSeverity_Error, indent, "Appearance's input is bound to a DataObject that does not exist");
                    return getValidationErrorStatus();
                }

                ObjectIteratorImpl iterator(getSceneImpl().getObjectRegistry(), ERamsesObjectType_DataObject);
                RamsesObject* ramsesObject = NULL;
                while (NULL != (ramsesObject = iterator.getNext()))
                {
                    const DataObject& dataObject = RamsesObjectTypeUtils::ConvertTo<DataObject>(*ramsesObject);
                    if (boundInstance == dataObject.impl.getDataReference())
                    {
                        const status_t dataObjectStatus = addValidationOfDependentObject(indent, dataObject.impl);
                        if (StatusOK != dataObjectStatus)
                        {
                            status = dataObjectStatus;
                        }
                        break;
                    }
                }
            }
        }

        return status;
    }

    void AppearanceImpl::cloneReferencedData(ramses_internal::IScene& scene, const AppearanceImpl& srcAppearance, AppearanceImpl& dstAppearance)
    {
        const ramses_internal::DataFieldHandle dataReferenceField(0u);
        dstAppearance.m_bindableInputs.reserve(srcAppearance.m_bindableInputs.count());
        for (const auto& srcBindableInput : srcAppearance.m_bindableInputs)
        {
            const ramses_internal::DataFieldHandle dataField(srcBindableInput.key);
            const ramses_internal::DataInstanceHandle srcDataRef = srcBindableInput.value.dataReference;
            const ramses_internal::DataLayoutHandle dataLayoutHandle = scene.getLayoutOfDataInstance(srcDataRef);
            const ramses_internal::EDataType dataType = scene.getDataLayout(dataLayoutHandle).getField(dataReferenceField).dataType;
            const bool isBound = srcBindableInput.value.externallyBound;

            // create internal data reference
            BindableInput bindableInput;
            bindableInput.externallyBound = isBound;
            bindableInput.dataReference = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dstAppearance.m_uniformInstance, dataField, dataType);
            dstAppearance.m_bindableInputs.put(srcBindableInput.key, bindableInput);

            if (isBound)
            {
                // set data reference in case it is externally bound in source appearance
                scene.setDataReference(dstAppearance.m_uniformInstance, dataField, scene.getDataReference(srcAppearance.m_uniformInstance, dataField));
            }
            else
            {
                // copy data in case it is not externally bound
                ramses_internal::DataInstanceHelper::CopyInstanceFieldData(scene, srcDataRef, dataReferenceField, scene, bindableInput.dataReference, dataReferenceField);
            }
        }
    }

    void AppearanceImpl::initializeFrameworkData(const EffectImpl& effect)
    {
        m_effectImpl = &effect;

        m_renderStateHandle = getIScene().allocateRenderState(ramses_internal::RenderStateHandle::Invalid());
        createUniformDataInstance(effect);
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
        m_uniformLayout = ramses_internal::DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(getIScene(), uniformsInputInfo, referencedInputs);
        m_uniformInstance = getIScene().allocateDataInstance(m_uniformLayout);

        m_bindableInputs.reserve(m_bindableInputs.count() + referencedInputs.size());
        for (const auto& refInput : referencedInputs)
        {
            const ramses_internal::DataFieldHandle dataField(refInput);

            BindableInput bindableInput;
            bindableInput.externallyBound = false;
            bindableInput.dataReference = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(getIScene(), m_uniformInstance, dataField, uniformsInputInfo[refInput].dataType);
            m_bindableInputs.put(refInput, bindableInput);
        }
    }

    status_t AppearanceImpl::checkEffectInputValidityAndValueCompatibility(const EffectInputImpl& input, uint32_t valueElementCount, ramses_internal::EDataType valueDataType) const
    {
        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            return addErrorEntry("Appearance::set failed, input is not properly initialized or cannot be used with this appearance.");
        }

        if (input.getDataType() != valueDataType)
        {
            return addErrorEntry("Appearance::set failed, value type does not match input data type");
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
        return setDataArrayChecked<ContainerT>(elementCount, reinterpret_cast<const ContainerT*>(valuesIn), input);
    }

    template <typename ContainerT, typename ElementT>
    status_t AppearanceImpl::getInputValueWithElementTypeCast(const EffectInputImpl& input, uint32_t elementCount, ElementT* valuesOut) const
    {
        return getDataArrayChecked<ContainerT>(elementCount, reinterpret_cast<ContainerT*>(valuesOut), input);
    }

    template <typename T>
    status_t AppearanceImpl::setDataArrayChecked(uint32_t elementCount, const T* values, const EffectInputImpl& input)
    {
        if (input.getSemantics() != ramses_internal::EFixedSemantics_Invalid)
        {
            return addErrorEntry("Appearance::set failed, can't access value of semantic uniform");
        }
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, elementCount, ramses_internal::TypeToEDataTypeTraits<T>::DataType));

        const BindableInput* bindableInput = m_bindableInputs.get(input.getInputIndex());
        const bool isBindable = (bindableInput != NULL);
        if (isBindable && bindableInput->externallyBound)
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
        if (input.getSemantics() != ramses_internal::EFixedSemantics_Invalid)
        {
            return addErrorEntry("Appearance::set failed, can't access value of semantic uniform");
        }
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, elementCount, ramses_internal::TypeToEDataTypeTraits<T>::DataType));

        const BindableInput* bindableInput = m_bindableInputs.get(input.getInputIndex());
        const bool isBindable = (bindableInput != NULL);
        if (isBindable && bindableInput->externallyBound)
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
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u, ramses_internal::EDataType_TextureSampler));

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

    status_t AppearanceImpl::bindInput(const EffectInputImpl& input, const DataObjectImpl& dataObject)
    {
        if (!isFromTheSameSceneAs(dataObject))
        {
            return addErrorEntry("Appearance::bindInput failed, dataObject is not from the same scene as this appearance");
        }

        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u, dataObject.getDataType()));

        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput == NULL)
        {
            return addErrorEntry("Appearance::bindInput failed, given uniform input cannot be bound to a DataObject.");
        }

        return bindInputInternal(input, dataObject);
    }

    status_t AppearanceImpl::unbindInput(const EffectInputImpl& input)
    {
        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        if (bindableInput == NULL || !bindableInput->externallyBound)
        {
            return addErrorEntry("Appearance::unbindInput failed, given uniform input is not bound to a DataObject.");
        }

        return unbindInputInternal(input);
    }

    bool AppearanceImpl::isInputBound(const EffectInputImpl& input) const
    {
        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        return (bindableInput != NULL) && bindableInput->externallyBound;
    }

    status_t AppearanceImpl::setInputTextureInternal(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler)
    {
        CHECK_RETURN_ERR(checkEffectInputValidityAndValueCompatibility(input, 1u, ramses_internal::EDataType_TextureSampler));

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
        assert(bindableInput != NULL);
        bindableInput->externallyBound = true;

        return StatusOK;
    }

    status_t AppearanceImpl::unbindInputInternal(const EffectInputImpl& input)
    {
        const uint32_t inputIndex = input.getInputIndex();
        BindableInput* bindableInput = m_bindableInputs.get(inputIndex);
        const ramses_internal::DataFieldHandle dataField(inputIndex);
        getIScene().setDataReference(m_uniformInstance, dataField, bindableInput->dataReference);
        bindableInput->externallyBound = false;

        return StatusOK;
    }

    ramses_internal::DataLayoutHandle AppearanceImpl::getUniformDataLayout() const
    {
        return m_uniformLayout;
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
