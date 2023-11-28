//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/ArrayBuffer.h"

#include "impl/GeometryImpl.h"
#include "impl/EffectInputImpl.h"
#include "impl/ArrayResourceImpl.h"
#include "impl/EffectImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SceneObjectImpl.h"
#include "impl/SceneImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "impl/DataTypeUtils.h"
#include "impl/ObjectIteratorImpl.h"
#include "impl/SerializationContext.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    GeometryImpl::GeometryImpl(SceneImpl& scene, std::string_view name)
        : SceneObjectImpl(scene, ERamsesObjectType::Geometry, name)
        , m_effectImpl(nullptr)
        , m_indicesCount(0u)
    {
    }

    GeometryImpl::~GeometryImpl() = default;

    void GeometryImpl::initializeFrameworkData(const EffectImpl& effect)
    {
        m_effectImpl = &effect;
        createDataLayout();
    }

    void GeometryImpl::deinitializeFrameworkData()
    {
        if (m_attributeInstance.isValid())
        {
            getIScene().releaseDataInstance(m_attributeInstance);
            m_attributeInstance = ramses::internal::DataInstanceHandle::Invalid();
        }

        if (m_attributeLayout.isValid())
        {
            getIScene().releaseDataLayout(m_attributeLayout);
            m_attributeLayout = ramses::internal::DataLayoutHandle::Invalid();
        }
    }

    bool GeometryImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << (m_effectImpl ? serializationContext.getIDForObject(m_effectImpl) : SerializationContext::GetObjectIDNull());

        outStream << m_attributeLayout;
        outStream << m_attributeInstance;
        outStream << m_indicesCount;

        return true;
    }

    bool GeometryImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_effectImpl);

        inStream >> m_attributeLayout;
        inStream >> m_attributeInstance;
        inStream >> m_indicesCount;

        serializationContext.addForDependencyResolve(this);

        return true;
    }

    bool GeometryImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_effectImpl);

        return true;
    }

    void GeometryImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);
        validateEffect(report);
        validateAttribute(report);
    }

    void GeometryImpl::validateEffect(ValidationReportImpl& report) const
    {
        ObjectIteratorImpl iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::Effect);
        RamsesObject* ramsesObject = iter.getNext();
        while (nullptr != ramsesObject)
        {
            const Effect& effect = RamsesObjectTypeUtils::ConvertTo<Effect>(*ramsesObject);
            if (&effect.impl() == m_effectImpl)
                return report.addDependentObject(*this, effect.impl());

            ramsesObject = iter.getNext();
        }

        return report.add(EIssueType::Error, "Geometry is referring to an invalid Effect", &getRamsesObject());
    }

    void GeometryImpl::validateAttribute(ValidationReportImpl& report) const
    {
        const ramses::internal::DataLayout& layout = getIScene().getDataLayout(m_attributeLayout);
        const uint32_t dataLayoutFieldCount = layout.getFieldCount();
        for (ramses::internal::DataFieldHandle fieldIndex(0u); fieldIndex < dataLayoutFieldCount; ++fieldIndex)
        {
            const ramses::internal::ResourceField& dataResource =
                getIScene().getDataResource(m_attributeInstance, fieldIndex);

            if (dataResource.dataBuffer.isValid())
            {
                const ramses::internal::EDataType fieldDataType = layout.getField(fieldIndex).dataType;
                validateDataBuffer(report, dataResource.dataBuffer, fieldDataType);
            }
            else
            {
                if (dataResource.hash.isValid())
                {
                    validateResource(report, dataResource.hash);
                }
            }
        }
    }

    void GeometryImpl::validateResource(ValidationReportImpl& report, ramses::internal::ResourceContentHash resourceHash) const
    {
        const Resource* resource = getSceneImpl().scanForResourceWithHash(resourceHash);
        if (nullptr == resource)
            return report.add(EIssueType::Error, "Geometry is referring to resource that does not exist", &getRamsesObject());
        return report.addDependentObject(*this, resource->impl());
    }

    void GeometryImpl::validateDataBuffer(ValidationReportImpl& report, ramses::internal::DataBufferHandle dataBuffer, ramses::internal::EDataType fieldDataType) const
    {
        if (!getIScene().isDataBufferAllocated(dataBuffer))
            return report.add(EIssueType::Error, "Geometry is referring to data buffer that does not exist", &getRamsesObject());

        const auto dataBufferType = getIScene().getDataBuffer(dataBuffer).dataType;
        if (!dataTypeMatchesInputType(dataBufferType, fieldDataType))
            return report.add(EIssueType::Error, "Geometry is referring to data buffer with type that does not match data layout field type", &getRamsesObject());
        const ArrayBufferImpl* dataBufferImpl = findDataBuffer(dataBuffer);
        assert(nullptr != dataBufferImpl);

        return report.addDependentObject(*this, *dataBufferImpl);
    }

    ArrayBufferImpl* GeometryImpl::findDataBuffer(ramses::internal::DataBufferHandle dataBufferHandle) const
    {
        SceneObjectRegistryIterator arrayBufferIter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::ArrayBuffer);
        while (auto* dataBuffer = arrayBufferIter.getNextNonConst<ArrayBuffer>())
        {
            if (dataBuffer->impl().getDataBufferHandle() == dataBufferHandle)
            {
                return &dataBuffer->impl();
            }
        }

        return nullptr;
    }

    void GeometryImpl::createDataLayout()
    {
        assert(!m_attributeLayout.isValid());

        const ramses::internal::EffectInputInformationVector& attributesList = m_effectImpl->getAttributeInputInformation();
        ramses::internal::DataFieldInfoVector dataFields;
        dataFields.reserve(attributesList.size());

        // Indices are always stored at fixed data slot with index IndicesDataFieldIndex
        dataFields.push_back(ramses::internal::DataFieldInfo{ ramses::internal::EDataType::Indices, 1u, ramses::internal::EFixedSemantics::Indices });
        for (const auto& attribInfo : attributesList)
        {
            dataFields.push_back(ramses::internal::DataFieldInfo{ attribInfo.dataType, attribInfo.elementCount, attribInfo.semantics });
        }

        m_attributeLayout   = getIScene().allocateDataLayout(dataFields, m_effectImpl->getLowlevelResourceHash(), {});
        m_attributeInstance = getIScene().allocateDataInstance(m_attributeLayout, {});
    }

    ramses::internal::ResourceContentHash GeometryImpl::getEffectHash() const
    {
        return m_effectImpl->getLowlevelResourceHash();
    }

    ramses::internal::DataLayoutHandle GeometryImpl::getAttributeDataLayout() const
    {
        return m_attributeLayout;
    }

    ramses::internal::DataInstanceHandle GeometryImpl::getAttributeDataInstance() const
    {
        return m_attributeInstance;
    }

    uint32_t GeometryImpl::getIndicesCount() const
    {
        return m_indicesCount;
    }

    bool GeometryImpl::setIndices(const ArrayResourceImpl& arrayResource)
    {
        if (!isFromTheSameSceneAs(arrayResource))
        {
            getErrorReporting().set("Geometry::setIndices failed, indicesResource is not from the same client as this Geometry.", *this);
            return false;
        }

        if (!DataTypeUtils::IsValidIndicesType(arrayResource.getElementType()))
        {
            getErrorReporting().set("Geometry::setIndices failed, indicesResource is not of valid data type.", *this);
            return false;
        }

        const ramses::internal::DataLayout& layout = getIScene().getDataLayout(m_attributeLayout);
        const uint32_t fieldCount = layout.getFieldCount();
        ramses::internal::EFixedSemantics indicesFieldSemantics = ramses::internal::EFixedSemantics::Invalid;
        const ramses::internal::DataFieldHandle field(IndicesDataFieldIndex);

        if (IndicesDataFieldIndex < fieldCount)
        {
            indicesFieldSemantics = layout.getField(field).semantics;
        }

        if (indicesFieldSemantics == ramses::internal::EFixedSemantics::Indices)
        {
            getIScene().setDataResource(m_attributeInstance, field, arrayResource.getLowlevelResourceHash(), ramses::internal::DataBufferHandle::Invalid(), 0u, 0u, 0u);
            m_indicesCount = static_cast<uint32_t>(arrayResource.getElementCount());

            return true;
        }

        getErrorReporting().set("Geometry::setIndices failed - indices slot was not enabled in this geometry.", *this);
        return false;
    }

    bool GeometryImpl::setIndices(const ArrayBufferImpl& dataBuffer)
    {
        if (!isFromTheSameSceneAs(dataBuffer))
        {
            getErrorReporting().set("Geometry::setIndices failed, dataBuffer is not from the same scene as this Geometry.", *this);
            return false;
        }

        if (!DataTypeUtils::IsValidIndicesType(dataBuffer.getDataType()))
        {
            getErrorReporting().set("Geometry::setIndices failed, arrayBuffer is not of valid data type.", *this);
            return false;
        }

        const ramses::internal::DataLayout& layout = getIScene().getDataLayout(m_attributeLayout);
        const uint32_t fieldCount = layout.getFieldCount();
        ramses::internal::EFixedSemantics indicesFieldSemantics = ramses::internal::EFixedSemantics::Invalid;
        const ramses::internal::DataFieldHandle field(IndicesDataFieldIndex);

        if (IndicesDataFieldIndex < fieldCount)
        {
            indicesFieldSemantics = layout.getField(field).semantics;
        }

        if (indicesFieldSemantics == ramses::internal::EFixedSemantics::Indices)
        {
            const ramses::internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();

            getIScene().setDataResource(m_attributeInstance, field, ramses::internal::ResourceContentHash::Invalid(), dataBufferHandle, 0u, 0u, 0u);

            m_indicesCount = static_cast<uint32_t>(dataBuffer.getElementCount());

            return true;
        }

        getErrorReporting().set("Geometry::setIndices failed - indices slot was not enabled in this geometry.", *this);
        return false;
    }

    bool GeometryImpl::setInputBuffer(const EffectInputImpl& input, const ArrayResourceImpl& bufferResource, uint32_t instancingDivisor, uint16_t offset, uint16_t stride)
    {
        if (!isFromTheSameSceneAs(bufferResource))
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, array resource is not from the same client as the Geometry.", *this);
            return false;
        }

        if (!DataTypeUtils::IsValidVerticesType(bufferResource.getElementType()))
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, array resource is not of valid data type.", *this);
            return false;
        }

        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, input is not properly initialized or cannot be used with this geometry binding.", *this);
            return false;
        }

        if ((offset > 0 || stride > 0) && bufferResource.getElementType() != ramses::EDataType::ByteBlob)
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, custom stride/offset can be used only with array resources of type byte blob", *this);
            return false;
        }

        if (!dataTypeMatchesInputType(DataTypeUtils::ConvertDataTypeToInternal(bufferResource.getElementType()), input.getInternalDataType()))
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, array resource type does not match input data type", *this);
            return false;
        }

        // data field index on low level scene is indexed starting after reserved slot for indices
        const ramses::internal::DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()) + IndicesDataFieldIndex + 1u);
        getIScene().setDataResource(m_attributeInstance, dataField, bufferResource.getLowlevelResourceHash(), ramses::internal::DataBufferHandle::Invalid(), instancingDivisor, offset, stride);

        return true;
    }

    bool GeometryImpl::setInputBuffer(const EffectInputImpl& input, const ArrayBufferImpl& dataBuffer, uint32_t instancingDivisor, uint16_t offset, uint16_t stride)
    {
        if (!isFromTheSameSceneAs(dataBuffer))
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, dataBuffer is not from the same scene as the Geometry.", *this);
            return false;
        }

        if (!DataTypeUtils::IsValidVerticesType(dataBuffer.getDataType()))
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, arrayBuffer is not of valid data type.", *this);
            return false;
        }

        if (input.getEffectHash() != m_effectImpl->getLowlevelResourceHash())
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, input is not properly initialized or cannot be used with this geometry binding.", *this);
            return false;
        }

        const ramses::internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();
        const ramses::internal::EDataType dataBufferDataType = getIScene().getDataBuffer(dataBufferHandle).dataType;

        if ((offset > 0 || stride > 0) && dataBuffer.getDataType() != ramses::EDataType::ByteBlob)
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, custom stride/offset can be used only with data buffers of type byte blob", *this);
            return false;
        }

        if (!dataTypeMatchesInputType(dataBufferDataType, input.getInternalDataType()))
        {
            getErrorReporting().set("Geometry::setInputBuffer failed, vertex data buffer type does not match input data type", *this);
            return false;
        }

        // data field index on low level scene is indexed starting after reserved slot for indices
        const ramses::internal::DataFieldHandle dataField(static_cast<uint32_t>(input.getInputIndex()) + IndicesDataFieldIndex + 1u);
        getIScene().setDataResource(m_attributeInstance, dataField, ramses::internal::ResourceContentHash::Invalid(), dataBufferHandle, instancingDivisor, offset, stride);

        return true;
    }

    bool GeometryImpl::dataTypeMatchesInputType(ramses::internal::EDataType resourceType, ramses::internal::EDataType inputDataType)
    {
        switch (resourceType)
        {
        case ramses::internal::EDataType::UInt16:
        case ramses::internal::EDataType::UInt32:
            return inputDataType == ramses::internal::EDataType::Indices;
        case ramses::internal::EDataType::ByteBlob:
            return ramses::internal::IsBufferDataType(inputDataType);
        case ramses::internal::EDataType::Float:
            return inputDataType == ramses::internal::EDataType::FloatBuffer;
        case ramses::internal::EDataType::Vector2F:
            return inputDataType == ramses::internal::EDataType::Vector2Buffer;
        case ramses::internal::EDataType::Vector3F:
            return inputDataType == ramses::internal::EDataType::Vector3Buffer;
        case ramses::internal::EDataType::Vector4F:
            return inputDataType == ramses::internal::EDataType::Vector4Buffer;
        default:
            return false;
        }
    }

    const Effect& GeometryImpl::getEffect() const
    {
        return RamsesObjectTypeUtils::ConvertTo<Effect>(m_effectImpl->getRamsesObject());
    }
}
