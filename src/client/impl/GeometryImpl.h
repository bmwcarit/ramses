//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// internal
#include "SceneObjectImpl.h"
#include "impl/ArrayResourceImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/EDataType.h"

#include <string_view>

namespace ramses
{
    class AttributeInput;
}

namespace ramses::internal
{
    class IScene;
    class ResourceImpl;
    class EffectImpl;
    class ArrayBufferImpl;
    class ArrayResourceImpl;

    class GeometryImpl final : public SceneObjectImpl
    {
    public:
        GeometryImpl(SceneImpl& scene, std::string_view name);
        ~GeometryImpl() override;

        void initializeFrameworkData(const EffectImpl& effect);
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        bool setInputBuffer(const EffectInputImpl& input, const ArrayResourceImpl& bufferResource, uint32_t instancingDivisor, uint16_t offset, uint16_t stride);
        bool setInputBuffer(const EffectInputImpl& input, const ArrayBufferImpl& dataBuffer, uint32_t instancingDivisor, uint16_t offset, uint16_t stride);
        bool setIndices(const ArrayResourceImpl& arrayResource);
        bool setIndices(const ArrayBufferImpl& dataBuffer);

        [[nodiscard]] ramses::internal::ResourceContentHash getEffectHash() const;
        [[nodiscard]] ramses::internal::DataLayoutHandle    getAttributeDataLayout() const;
        [[nodiscard]] ramses::internal::DataInstanceHandle  getAttributeDataInstance() const;
        [[nodiscard]] uint32_t                             getIndicesCount() const;

        [[nodiscard]] const Effect& getEffect() const;

        static const uint32_t IndicesDataFieldIndex = 0u;

    private:
        void createDataLayout();

        void validateEffect(ValidationReportImpl& report) const;
        void validateAttribute(ValidationReportImpl& report) const;
        void validateResource(ValidationReportImpl& report, ramses::internal::ResourceContentHash resourceHash) const;
        void validateDataBuffer(ValidationReportImpl& report, ramses::internal::DataBufferHandle dataBuffer, ramses::internal::EDataType fieldDataType) const;

        [[nodiscard]] ArrayBufferImpl* findDataBuffer(ramses::internal::DataBufferHandle dataBufferHandle) const;

        static bool dataTypeMatchesInputType(ramses::internal::EDataType resourceType, ramses::internal::EDataType inputDataType);

        const EffectImpl*                       m_effectImpl;
        ramses::internal::DataLayoutHandle       m_attributeLayout;
        ramses::internal::DataInstanceHandle     m_attributeInstance;
        uint32_t                                m_indicesCount;
    };
}
