//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/ArrayBuffer.h"

#include "GeometryBindingImpl.h"
#include "EffectInputImpl.h"
#include "ArrayResourceImpl.h"
#include "EffectImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "Scene/ClientScene.h"
#include "SceneObjectImpl.h"
#include "SceneImpl.h"
#include "ArrayBufferImpl.h"
#include "RamsesObjectRegistryIterator.h"
#include "ArrayResourceUtils.h"
#include "ObjectIteratorImpl.h"
#include "SerializationContext.h"

namespace ramses
{
    GeometryBindingImpl::GeometryBindingImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_GeometryBinding, name)
        , m_effectImpl(nullptr)
        , m_indicesCount(0u)
    {
    }

    GeometryBindingImpl::~GeometryBindingImpl()
    {
    }

    void GeometryBindingImpl::initializeFrameworkData(const EffectImpl& effect)
    {
        m_effectImpl = &effect;
        createDataLayout();
    }

    void GeometryBindingImpl::deinitializeFrameworkData()
    {
        if (m_attributeInstance.isValid())
        {
            getIScene().releaseDataInstance(m_attributeInstance);
            m_attributeInstance = ramses_internal::DataInstanceHandle::Invalid();
        }

        if (m_attributeLayout.isValid())
        {
            getIScene().releaseDataLayout(m_attributeLayout);
            m_attributeLayout = ramses_internal::DataLayoutHandle::Invalid();
        }
    }

    status_t GeometryBindingImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << (m_effectImpl ? serializationContext.getIDForObject(m_effectImpl) : serializationContext.GetObjectIDNull());

        outStream << m_attributeLayout;
        outStream << m_attributeInstance;
        outStream << m_indicesCount;

        return StatusOK;
    }

    status_t GeometryBindingImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_effectImpl);

        inStream >> m_attributeLayout;
        inStream >> m_attributeInstance;
        inStream >> m_indicesCount;

        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t GeometryBindingImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_effectImpl);

        return StatusOK;
    }

    status_t GeometryBindingImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = SceneObjectImpl::validate(indent, visitedObjects);

        const status_t effectStatus = validateEffect(indent, visitedObjects);
        if (StatusOK != effectStatus)
        {
            status = effectStatus;
        }

        const status_t attributeStatus = validateAttribute(indent, visitedObjects);
        if (StatusOK != attributeStatus)
        {
            status = attributeStatus;
        }

        return status;
    }

    status_t GeometryBindingImpl::validateEffect(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        ObjectIteratorImpl iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_Effect);
        RamsesObject* ramsesObject = iter.getNext();
        while (nullptr != ramsesObject)
        {
            const Effect& effect = RamsesObjectTypeUtils::ConvertTo<Effect>(*ramsesObject);
            if (&effect.impl == m_effectImpl)
            {
                return addValidationOfDependentObject(indent, *m_effectImpl, visitedObjects);
            }

            ramsesObject = iter.getNext();
        }

        addValidationMessage(EValidationSeverity_Error, indent, "GeometryBinding is referring to an invalid Effect");
        return getValidationErrorStatus();
    }

    status_t GeometryBindingImpl::validateAttribute(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = StatusOK;
        const ramses_internal::DataLayout& layout = getIScene().getDataLayout(m_attributeLayout);
        const uint32_t dataLayoutFieldCount = layout.getFieldCount();
        for (ramses_internal::DataFieldHandle fieldIndex(0u); fieldIndex < dataLayoutFieldCount; ++fieldIndex)
        {
            const ramses_internal::ResourceField& dataResource =
                getIScene().getDataResource(m_attributeInstance, fieldIndex);

            if (dataResource.dataBuffer.isValid())
            {
                const ramses_internal::EDataType fieldDataType = layout.getField(fieldIndex).dataType;
                const status_t dataBufferStatus =
                    validateDataBuffer(indent, dataResource.dataBuffer, fieldDataType, visitedObjects);
                if (StatusOK != dataBufferStatus)
                {
                    status = dataBufferStatus;
                }
            }
            else
            {
                if (dataResource.hash.isValid())
                {
                    const status_t resourceStatus = validateResource(indent, dataResource.hash, visitedObjects);
                    if (StatusOK != resourceStatus)
                    {
                        status = resourceStatus;
                    }
                }
            }
        }

        return status;
    }

    status_t GeometryBindingImpl::validateResource(uint32_t indent, ramses_internal::ResourceContentHash resourceHash, StatusObjectSet& visitedObjects) const
    {
        const Resource* resource = getSceneImpl().scanForResourceWithHash(resourceHash);
        if (nullptr == resource)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "GeometryBinding is referring to resource that does not exist");
            return getValidationErrorStatus();
        }

        return addValidationOfDependentObject(indent, resource->impl, visitedObjects);
    }

    status_t GeometryBindingImpl::validateDataBuffer(uint32_t indent, ramses_internal::DataBufferHandle dataBuffer, ramses_internal::EDataType fieldDataType, StatusObjectSet& visitedObjects) const
    {
        if (!getIScene().isDataBufferAllocated(dataBuffer))
        {
            addValidationMessage(EValidationSeverity_Error, indent, "GeometryBinding is referring to data buffer that does not exist");
            return getValidationErrorStatus();
        }

        if (!dataTypeMatchesInputType(getIScene().getDataBuffer(dataBuffer).dataType, fieldDataType))
        {
            addValidationMessage(EValidationSeverity_Error, indent, "GeometryBinding is referring to data buffer with type that does not match data layout field type");
            return getValidationErrorStatus();
        }

        const ArrayBufferImpl* dataBufferImpl = findDataBuffer(dataBuffer);
        assert(nullptr != dataBufferImpl);

        return addValidationOfDependentObject(indent, *dataBufferImpl, visitedObjects);
    }

    ramses::ArrayBufferImpl* GeometryBindingImpl::findDataBuffer(ramses_internal::DataBufferHandle dataBufferHandle) const
    {
        RamsesObjectRegistryIterator arrayBufferIter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_DataBufferObject);
        while (const ArrayBuffer* dataBuffer = arrayBufferIter.getNext<ArrayBuffer>())
        {
            if (dataBuffer->impl.getDataBufferHandle() == dataBufferHandle)
            {
                return &dataBuffer->impl;
            }
        }

        return nullptr;
    }

    void GeometryBindingImpl::createDataLayout()
    {
        assert(!m_attributeLayout.isValid());

        const ramses_internal::EffectInputInformationVector& attributesList = m_effectImpl->getAttributeInputInformation();
        ramses_internal::DataFieldInfoVector dataFields;
        dataFields.reserve(attributesList.size());

        // Indices are always stored at fixed data slot with index IndicesDataFieldIndex
        dataFields.push_back(ramses_internal::DataFieldInfo{ ramses_internal::EDataType::Indices, 1u, ramses_internal::EFixedSemantics_Indices });
        for (const auto& attribInfo : attributesList)
        {
            dataFields.push_back(ramses_internal::DataFieldInfo{ attribInfo.dataType, attribInfo.elementCount, attribInfo.semantics });
        }

        m_attributeLayout = getIScene().allocateDataLayout(dataFields, m_effectImpl->getLowlevelResourceHash());
        m_attributeInstance = getIScene().allocateDataInstance(m_attributeLayout);
    }

    ramses_internal::ResourceContentHash GeometryBindingImpl::getEffectHash() const
    {
        return m_effectImpl->getLowlevelResourceHash();
    }

    ramses_internal::DataLayoutHandle GeometryBindingImpl::getAttributeDataLayout() const
    {
        return m_attributeLayout;
    }

    ramses_internal::DataInstanceHandle GeometryBindingImpl::getAttributeDataInstance() const
    {
        return m_attributeInstance;
    }

    uint32_t GeometryBindingImpl::getIndicesCount() const
    {
        return m_indicesCount;
    }

    status_t GeometryBindingImpl::setIndices(const ArrayResourceImpl& arrayResource)
    {
        if (!isFromTheSameSceneAs(arrayResource))
        {
            return addErrorEntry("GeometryBinding::setIndices failed, indicesResource is not from the same client as this GeometryBinding.");
        }

        if (!ArrayResourceUtils::IsValidIndicesType(arrayResource.getElementType()))
        {
            return addErrorEntry("GeometryBinding::setIndices failed, indicesResource is not of valid data type.");
        }

        const ramses_internal::DataLayout& layout = getIScene().getDataLayout(m_attributeLayout);
        const uint32_t fieldCount = layout.getFieldCount();
        ramses_internal::EFixedSemantics indicesFieldSemantics = ramses_internal::EFixedSemantics_Invalid;
        const ramses_internal::DataFieldHandle field(IndicesDataFieldIndex);

        if (IndicesDataFieldIndex < fieldCount)
        {
            indicesFieldSemantics = layout.getField(field).semantics;
        }

        if (indicesFieldSemantics == ramses_internal::EFixedSemantics_Indices)
        {
            getIScene().setDataResource(m_attributeInstance, field, arrayResource.getLowlevelResourceHash(), ramses_internal::DataBufferHandle::Invalid(), 0);
            m_indicesCount = arrayResource.getElementCount();

            return StatusOK;
        }

        return addErrorEntry("GeometryBinding::setIndices failed - indices slot was not enabled in this geometry.");
    }

    status_t GeometryBindingImpl::setIndices(const ArrayBufferImpl& dataBuffer)
    {
        if (!isFromTheSameSceneAs(dataBuffer))
        {
            return addErrorEntry("GeometryBinding::setIndices failed, dataBuffer is not from the same scene as this GeometryBinding.");
        }

        if (!ArrayResourceUtils::IsValidIndicesType(dataBuffer.getDataType()))
        {
            return addErrorEntry("GeometryBinding::setIndices failed, arrayBuffer is not of valid data type.");
        }

        const ramses_internal::DataLayout& layout = getIScene().getDataLayout(m_attributeLayout);
        const uint32_t fieldCount = layout.getFieldCount();
        ramses_internal::EFixedSemantics indicesFieldSemantics = ramses_internal::EFixedSemantics_Invalid;
        const ramses_internal::DataFieldHandle field(IndicesDataFieldIndex);

        if (IndicesDataFieldIndex < fieldCount)
        {
            indicesFieldSemantics = layout.getField(field).semantics;
        }

        if (indicesFieldSemantics == ramses_internal::EFixedSemantics_Indices)
        {
            const ramses_internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();

            getIScene().setDataResource(m_attributeInstance, field, ramses_internal::ResourceContentHash::Invalid(), dataBufferHandle, 0);

            m_indicesCount = dataBuffer.getElementCount();

            return StatusOK;
        }

        return addErrorEntry("GeometryBinding::setIndices failed - indices slot was not enabled in this geometry.");
    }

    status_t GeometryBindingImpl::setInputBuffer(const EffectInputImpl& input, const ArrayResourceImpl& bufferResource, uint32_t instancingDivisor)
    {
        if (!isFromTheSameSceneAs(bufferResource))
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, bufferResource is not from the same client as the GeometryBinding.");
        }

        if (!ArrayResourceUtils::IsValidVerticesType(bufferResource.getElementType()))
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, bufferResource is not of valid data type.");
        }

        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, input is not properly initialized or cannot be used with this geometry binding.");
        }

        if (!dataTypeMatchesInputType(ArrayResourceUtils::ConvertDataTypeForResourceToInternal(bufferResource.getElementType()), input.getDataType()))
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, resource buffer type does not match input data type");
        }

        // data field index on low level scene is indexed starting after reserved slot for indices
        const ramses_internal::DataFieldHandle dataField(input.getInputIndex() + IndicesDataFieldIndex + 1u);
        getIScene().setDataResource(m_attributeInstance, dataField, bufferResource.getLowlevelResourceHash(), ramses_internal::DataBufferHandle::Invalid(), instancingDivisor);

        return StatusOK;
    }

    status_t GeometryBindingImpl::setInputBuffer(const EffectInputImpl& input, const ArrayBufferImpl& dataBuffer, uint32_t instancingDivisor /*= 0*/)
    {
        if (!isFromTheSameSceneAs(dataBuffer))
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, dataBuffer is not from the same scene as the GeometryBinding.");
        }

        if (!ArrayResourceUtils::IsValidVerticesType(dataBuffer.getDataType()))
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, arrayBuffer is not of valid data type.");
        }

        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, input is not properly initialized or cannot be used with this geometry binding.");
        }

        const ramses_internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();
        const ramses_internal::EDataType dataBufferDataType = getIScene().getDataBuffer(dataBufferHandle).dataType;

        if (!dataTypeMatchesInputType(dataBufferDataType, input.getDataType()))
        {
            return addErrorEntry("GeometryBinding::setInputBuffer failed, vertex data buffer type does not match input data type");
        }

        // data field index on low level scene is indexed starting after reserved slot for indices
        const ramses_internal::DataFieldHandle dataField(input.getInputIndex() + IndicesDataFieldIndex + 1u);
        getIScene().setDataResource(m_attributeInstance, dataField, ramses_internal::ResourceContentHash::Invalid(), dataBufferHandle, instancingDivisor);

        return StatusOK;
    }

    bool GeometryBindingImpl::dataTypeMatchesInputType(ramses_internal::EDataType resourceType, ramses_internal::EDataType inputDataType)
    {
        switch (resourceType)
        {
        case ramses_internal::EDataType::UInt16:
        case ramses_internal::EDataType::UInt32:
            return inputDataType == ramses_internal::EDataType::Indices;
        case ramses_internal::EDataType::Float:
            return inputDataType == ramses_internal::EDataType::FloatBuffer;
        case ramses_internal::EDataType::Vector2F:
            return inputDataType == ramses_internal::EDataType::Vector2Buffer;
        case ramses_internal::EDataType::Vector3F:
            return inputDataType == ramses_internal::EDataType::Vector3Buffer;
        case ramses_internal::EDataType::Vector4F:
            return inputDataType == ramses_internal::EDataType::Vector4Buffer;
        default:
            return false;
        }
    }

    const Effect& GeometryBindingImpl::getEffect() const
    {
        return RamsesObjectTypeUtils::ConvertTo<Effect>(m_effectImpl->getRamsesObject());
    }
}
