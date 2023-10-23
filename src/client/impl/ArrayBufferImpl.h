//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneObjectImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "ramses/framework/EDataType.h"

#include <string_view>

namespace ramses::internal
{
    class ArrayBufferImpl : public SceneObjectImpl
    {
    public:
        ArrayBufferImpl(SceneImpl& scene, std::string_view databufferName);
        ~ArrayBufferImpl() override;

        void initializeFrameworkData(ramses::EDataType dataType, uint32_t numElements);
        void deinitializeFrameworkData() override;
        bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext) override;

        void onValidate(ValidationReportImpl& report) const override;

        bool updateData(uint32_t firstElement, uint32_t numElements, const std::byte* bufferData);

        [[nodiscard]] DataBufferHandle getDataBufferHandle() const;
        [[nodiscard]] uint32_t getMaximumNumberOfElements() const;
        [[nodiscard]] uint32_t getElementCount() const;
        [[nodiscard]] uint32_t getUsedNumberOfElements() const;
        [[nodiscard]] uint32_t getUsedElementCount() const;
        [[nodiscard]] ramses::EDataType getDataType() const;
        [[nodiscard]] bool getData(std::byte* buffer, uint32_t numElements) const;

    private:
        DataBufferHandle m_dataBufferHandle;
    };
}
