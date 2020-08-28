//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ArrayBufferImpl.h"
#include "SerializationContext.h"
#include "DataTypeUtils.h"
#include "Scene/ClientScene.h"
#include "SceneAPI/EDataType.h"
#include "ArrayResourceUtils.h"

namespace ramses
{
    ArrayBufferImpl::ArrayBufferImpl(SceneImpl& scene, const char* databufferName)
        : SceneObjectImpl(scene, ERamsesObjectType_DataBufferObject, databufferName)
    {
    }

    ArrayBufferImpl::~ArrayBufferImpl()
    {
    }

    void ArrayBufferImpl::initializeFrameworkData(EDataType dataType, uint32_t numElements)
    {
        assert(!m_dataBufferHandle.isValid());
        const ramses_internal::EDataBufferType dataBufferType = ArrayResourceUtils::DeductBufferTypeFromDataType(dataType);
        const ramses_internal::EDataType dataTypeInternal = DataTypeUtils::GetDataTypeInternal(dataType);
        const uint32_t maximumSizeInBytes = EnumToSize(dataTypeInternal) * numElements;
        m_dataBufferHandle = getIScene().allocateDataBuffer(dataBufferType, dataTypeInternal, maximumSizeInBytes);
    }

    void ArrayBufferImpl::deinitializeFrameworkData()
    {
        assert(m_dataBufferHandle.isValid());
        getIScene().releaseDataBuffer(m_dataBufferHandle);
        m_dataBufferHandle = ramses_internal::DataBufferHandle::Invalid();
    }


    ramses_internal::DataBufferHandle ArrayBufferImpl::getDataBufferHandle() const
    {
        return m_dataBufferHandle;
    }

    uint32_t ArrayBufferImpl::getElementCount() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return static_cast<uint32_t>(dataBuffer.data.size()) / EnumToSize(dataBuffer.dataType);
    }

    uint32_t ArrayBufferImpl::getUsedElementCount() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return static_cast<uint32_t>(dataBuffer.usedSize) / EnumToSize(dataBuffer.dataType);
    }

    EDataType ArrayBufferImpl::getDataType() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return  DataTypeUtils::GetDataTypeFromInternal(dataBuffer.dataType);
    }

    status_t ArrayBufferImpl::getData(ramses_internal::Byte* buffer, uint32_t numElements) const
    {
        const auto& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        const uint32_t dataSizeToCopy = std::min<uint32_t>(numElements * EnumToSize(dataBuffer.dataType), static_cast<uint32_t>(dataBuffer.data.size()));
        ramses_internal::PlatformMemory::Copy(buffer, dataBuffer.data.data(), dataSizeToCopy);

        return StatusOK;
    }

    status_t ArrayBufferImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_dataBufferHandle;

        return StatusOK;
    }

    status_t ArrayBufferImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_dataBufferHandle;

        return StatusOK;
    }

    status_t ArrayBufferImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = SceneObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        const auto& iscene = getIScene();

        const bool isInitialized = (iscene.getDataBuffer(getDataBufferHandle()).usedSize > 0u);

        bool usedAsInput = false;
        for (ramses_internal::DataInstanceHandle di(0u); di < iscene.getDataInstanceCount() && !usedAsInput; ++di)
        {
            if (!iscene.isDataInstanceAllocated(di))
                continue;

            const auto dlh = iscene.getLayoutOfDataInstance(di);
            const ramses_internal::DataLayout& dl = iscene.getDataLayout(dlh);
            for (ramses_internal::DataFieldHandle df(0u); df < dl.getFieldCount(); ++df)
            {
                switch (dl.getField(df).dataType)
                {
                case ramses_internal::EDataType::Indices:
                case ramses_internal::EDataType::UInt16Buffer:
                case ramses_internal::EDataType::FloatBuffer:
                case ramses_internal::EDataType::Vector2Buffer:
                case ramses_internal::EDataType::Vector3Buffer:
                case ramses_internal::EDataType::Vector4Buffer:
                {
                    const auto& resource = iscene.getDataResource(di, df);
                    if (resource.dataBuffer == getDataBufferHandle())
                        usedAsInput = true;
                    break;
                }
                default:
                    break;
                }
            }
        }

        for (ramses_internal::PickableObjectHandle po(0u); po < iscene.getPickableObjectCount() && !usedAsInput; ++po)
        {
            if (iscene.isPickableObjectAllocated(po) && iscene.getPickableObject(po).geometryHandle == getDataBufferHandle())
                usedAsInput = true;
        }

        if (usedAsInput && !isInitialized)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "DataBuffer is used as geometry input but there is no data set, this could lead to graphical glitches if actually rendered.");
            return getValidationErrorStatus();
        }

        if (!usedAsInput)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "DataBuffer is not used anywhere, destroy it if not needed.");
            return getValidationErrorStatus();
        }

        return status;
    }

    status_t ArrayBufferImpl::updateData(uint32_t firstElement, uint32_t numElements, const void* bufferData)
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        const size_t maximumSizeInBytes = dataBuffer.data.size();
        const uint32_t offsetInBytes = firstElement * EnumToSize(dataBuffer.dataType);
        const uint32_t dataSizeInBytes = numElements * EnumToSize(dataBuffer.dataType);
        if (offsetInBytes + dataSizeInBytes > maximumSizeInBytes)
        {
            return addErrorEntry("DataBuffer::update failed - trying to write data beyond maximum size");
        }

        getIScene().updateDataBuffer(m_dataBufferHandle, offsetInBytes, dataSizeInBytes, static_cast<const ramses_internal::Byte*>(bufferData));

        return StatusOK;
    }

    uint32_t ArrayBufferImpl::getMaximumNumberOfElements() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return static_cast<uint32_t>(dataBuffer.data.size()) / EnumToSize(dataBuffer.dataType);
    }

    uint32_t ArrayBufferImpl::getUsedNumberOfElements() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return dataBuffer.usedSize / EnumToSize(dataBuffer.dataType);
    }
}
