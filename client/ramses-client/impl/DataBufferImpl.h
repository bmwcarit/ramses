//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATABUFFERIMPL_H
#define RAMSES_DATABUFFERIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/EDataType.h"

namespace ramses
{
    class DataBufferImpl : public SceneObjectImpl
    {
    public:
        DataBufferImpl(SceneImpl& scene, ERamsesObjectType ramsesObjectBuffertype, const char* databufferName);
        virtual ~DataBufferImpl();

        void             initializeFrameworkData(uint32_t maximumSizeInBytes, EDataType dataType);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        virtual status_t validate(uint32_t indent) const override;

        status_t setData(const ramses_internal::Byte* data, uint32_t dataSizeInBytes, uint32_t offsetInBytes);

        ramses_internal::DataBufferHandle getDataBufferHandle() const;
        uint32_t getMaximumSizeInBytes() const;
        uint32_t getElementCount() const;
        uint32_t getUsedSizeInBytes() const;
        uint32_t getUsedElementCount() const;
        EDataType getDataType() const;
        status_t getData(ramses_internal::Byte* buffer, uint32_t bufferSize) const;

    private:
        ramses_internal::DataBufferHandle m_dataBufferHandle;
    };
}

#endif
