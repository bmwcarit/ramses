//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/ArrayBufferImpl.h"
#include "impl/SerializationContext.h"
#include "impl/DataTypeUtils.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"

namespace ramses::internal
{
    ArrayBufferImpl::ArrayBufferImpl(SceneImpl& scene, std::string_view databufferName)
        : SceneObjectImpl(scene, ERamsesObjectType::ArrayBuffer, databufferName)
    {
    }

    ArrayBufferImpl::~ArrayBufferImpl() = default;

    void ArrayBufferImpl::initializeFrameworkData(ramses::EDataType dataType, size_t numElements)
    {
        assert(!m_dataBufferHandle.isValid());
        const ramses::internal::EDataBufferType dataBufferType = DataTypeUtils::DeductBufferTypeFromDataType(dataType);
        const ramses::internal::EDataType dataTypeInternal = DataTypeUtils::ConvertDataTypeToInternal(dataType);
        const uint32_t maximumSizeInBytes = EnumToSize(dataTypeInternal) * static_cast<uint32_t>(numElements);
        m_dataBufferHandle = getIScene().allocateDataBuffer(dataBufferType, dataTypeInternal, maximumSizeInBytes, {});
    }

    void ArrayBufferImpl::deinitializeFrameworkData()
    {
        assert(m_dataBufferHandle.isValid());
        getIScene().releaseDataBuffer(m_dataBufferHandle);
        m_dataBufferHandle = DataBufferHandle::Invalid();
    }


    DataBufferHandle ArrayBufferImpl::getDataBufferHandle() const
    {
        return m_dataBufferHandle;
    }

    size_t ArrayBufferImpl::getElementCount() const
    {
        const GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return dataBuffer.data.size() / EnumToSize(dataBuffer.dataType);
    }

    size_t ArrayBufferImpl::getUsedElementCount() const
    {
        const GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return dataBuffer.usedSize / EnumToSize(dataBuffer.dataType);
    }

    ramses::EDataType ArrayBufferImpl::getDataType() const
    {
        const GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return DataTypeUtils::ConvertDataTypeFromInternal(dataBuffer.dataType);
    }

    bool ArrayBufferImpl::getData(std::byte* buffer, size_t numElements) const
    {
        const auto& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        const auto dataSizeToCopy = std::min<size_t>(numElements * EnumToSize(dataBuffer.dataType), dataBuffer.data.size());
        PlatformMemory::Copy(buffer, dataBuffer.data.data(), dataSizeToCopy);

        return true;
    }

    bool ArrayBufferImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_dataBufferHandle;

        return true;
    }

    bool ArrayBufferImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_dataBufferHandle;

        return true;
    }

    void ArrayBufferImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        const auto& iscene = getIScene();

        const bool isInitialized = (iscene.getDataBuffer(getDataBufferHandle()).usedSize > 0u);

        bool usedAsInput = false;
        for (DataInstanceHandle di(0u); di < iscene.getDataInstanceCount() && !usedAsInput; ++di)
        {
            if (!iscene.isDataInstanceAllocated(di))
                continue;

            const auto dlh = iscene.getLayoutOfDataInstance(di);
            const DataLayout& dl = iscene.getDataLayout(dlh);
            for (DataFieldHandle df(0u); df < dl.getFieldCount(); ++df)
            {
                switch (dl.getField(df).dataType)
                {
                case ramses::internal::EDataType::Indices:
                case ramses::internal::EDataType::UInt16Buffer:
                case ramses::internal::EDataType::FloatBuffer:
                case ramses::internal::EDataType::Vector2Buffer:
                case ramses::internal::EDataType::Vector3Buffer:
                case ramses::internal::EDataType::Vector4Buffer:
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

        for (PickableObjectHandle po(0u); po < iscene.getPickableObjectCount() && !usedAsInput; ++po)
        {
            if (iscene.isPickableObjectAllocated(po) && iscene.getPickableObject(po).geometryHandle == getDataBufferHandle())
                usedAsInput = true;
        }

        if (usedAsInput && !isInitialized)
            report.add(EIssueType::Warning, "DataBuffer is used as geometry input but there is no data set, this could lead to graphical glitches if actually rendered.", &getRamsesObject());

        if (!usedAsInput)
            report.add(EIssueType::Warning, "DataBuffer is not used anywhere, destroy it if not needed.", &getRamsesObject());
    }

    bool ArrayBufferImpl::updateData(size_t firstElement, size_t numElements, const std::byte* bufferData)
    {
        const GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        const size_t maximumSizeInBytes = dataBuffer.data.size();
        const size_t offsetInBytes = firstElement * EnumToSize(dataBuffer.dataType);
        const size_t dataSizeInBytes = numElements * EnumToSize(dataBuffer.dataType);
        if (offsetInBytes + dataSizeInBytes > maximumSizeInBytes)
        {
            getErrorReporting().set("DataBuffer::update failed - trying to write data beyond maximum size", *this);
            return false;
        }

        getIScene().updateDataBuffer(m_dataBufferHandle, static_cast<uint32_t>(offsetInBytes), static_cast<uint32_t>(dataSizeInBytes), bufferData);

        return true;
    }

    size_t ArrayBufferImpl::getMaximumNumberOfElements() const
    {
        const GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return dataBuffer.data.size() / EnumToSize(dataBuffer.dataType);
    }

    size_t ArrayBufferImpl::getUsedNumberOfElements() const
    {
        const GeometryDataBuffer& dataBuffer = getIScene().getDataBuffer(m_dataBufferHandle);
        return dataBuffer.usedSize / EnumToSize(dataBuffer.dataType);
    }
}
