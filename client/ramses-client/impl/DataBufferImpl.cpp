//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DataBufferImpl.h"
#include "SerializationContext.h"
#include "DataTypeUtils.h"
#include "Scene/ClientScene.h"
#include "SceneAPI/EDataType.h"

namespace ramses
{
    DataBufferImpl::DataBufferImpl(SceneImpl& scene, ERamsesObjectType ramsesObjectBuffertype, const char* databufferName)
        : SceneObjectImpl(scene, ramsesObjectBuffertype, databufferName)
    {
        assert(ERamsesObjectType_VertexDataBuffer == ramsesObjectBuffertype || ERamsesObjectType_IndexDataBuffer == ramsesObjectBuffertype);
    }

    DataBufferImpl::~DataBufferImpl()
    {
    }

    void DataBufferImpl::initializeFrameworkData(uint32_t maximumSizeInBytes, EDataType dataType)
    {
        assert(!m_dataBufferHandle.isValid());
        const ramses_internal::EDataBufferType dataBufferType = (ERamsesObjectType_VertexDataBuffer == getType()) ? ramses_internal::EDataBufferType::VertexBuffer : ramses_internal::EDataBufferType::IndexBuffer;
        const ramses_internal::EDataType dataTypeInternal = DataTypeUtils::GetDataTypeInternal(dataType);
        m_dataBufferHandle = getIScene().allocateDataBuffer(dataBufferType, dataTypeInternal, maximumSizeInBytes);
    }

    void DataBufferImpl::deinitializeFrameworkData()
    {
        assert(m_dataBufferHandle.isValid());
        getIScene().releaseDataBuffer(m_dataBufferHandle);
        m_dataBufferHandle = ramses_internal::DataBufferHandle::Invalid();
    }


    ramses_internal::DataBufferHandle DataBufferImpl::getDataBufferHandle() const
    {
        return m_dataBufferHandle;
    }

    uint32_t DataBufferImpl::getElementCount() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return static_cast<uint32_t>(dataBuffer.data.size()) / EnumToSize(dataBuffer.dataType);
    }

    uint32_t DataBufferImpl::getUsedElementCount() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return static_cast<uint32_t>(dataBuffer.usedSize) / EnumToSize(dataBuffer.dataType);
    }

    EDataType DataBufferImpl::getDataType() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return  DataTypeUtils::GetDataTypeFromInternal(dataBuffer.dataType);
    }

    status_t DataBufferImpl::getData(ramses_internal::Byte* buffer, uint32_t bufferSize) const
    {
        const auto& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle).data;
        const uint32_t dataSizeToCopy = std::min<uint32_t>(bufferSize, static_cast<uint32_t>(dataBuffer.size()));
        ramses_internal::PlatformMemory::Copy(buffer, dataBuffer.data(), dataSizeToCopy);

        return StatusOK;
    }

    status_t DataBufferImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_dataBufferHandle;

        return StatusOK;
    }

    status_t DataBufferImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_dataBufferHandle;

        return StatusOK;
    }

    status_t DataBufferImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
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
                case ramses_internal::EDataType_Indices:
                case ramses_internal::EDataType_UInt16Buffer:
                case ramses_internal::EDataType_FloatBuffer:
                case ramses_internal::EDataType_Vector2Buffer:
                case ramses_internal::EDataType_Vector3Buffer:
                case ramses_internal::EDataType_Vector4Buffer:
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

    status_t DataBufferImpl::setData(const ramses_internal::Byte* data, uint32_t dataSizeInBytes, uint32_t offsetInBytes)
    {
        const size_t maximumSize = getIScene().getDataBuffer(m_dataBufferHandle).data.size();
        if (offsetInBytes + dataSizeInBytes > maximumSize)
        {
            return addErrorEntry("DataBuffer::update failed - trying to write data beyond maximum size");
        }

        getIScene().updateDataBuffer(m_dataBufferHandle, offsetInBytes, dataSizeInBytes, data);

        return StatusOK;
    }

    uint32_t DataBufferImpl::getMaximumSizeInBytes() const
    {
        const ramses_internal::GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return static_cast<uint32_t>(dataBuffer.data.size());
    }

    uint32_t DataBufferImpl::getUsedSizeInBytes() const
    {
        return getIScene().getDataBuffer(m_dataBufferHandle).usedSize;
    }
}
