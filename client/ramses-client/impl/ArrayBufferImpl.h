//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYBUFFERIMPL_H
#define RAMSES_ARRAYBUFFERIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/EDataType.h"

namespace ramses
{
    class ArrayBufferImpl : public SceneObjectImpl
    {
    public:
        ArrayBufferImpl(SceneImpl& scene, const char* databufferName);
        virtual ~ArrayBufferImpl() override;

        void             initializeFrameworkData(EDataType dataType, uint32_t numElements);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        virtual status_t validate() const override;

        status_t updateData(uint32_t firstElement, uint32_t numElements, const void* bufferData);

        ramses_internal::DataBufferHandle getDataBufferHandle() const;
        uint32_t getMaximumNumberOfElements() const;
        uint32_t getElementCount() const;
        uint32_t getUsedNumberOfElements() const;
        uint32_t getUsedElementCount() const;
        EDataType getDataType() const;
        status_t getData(ramses_internal::Byte* buffer, uint32_t numElements) const;

    private:
        ramses_internal::DataBufferHandle m_dataBufferHandle;
    };
}

#endif
