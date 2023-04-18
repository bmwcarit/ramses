//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GEOMETRYBINDINGIMPL_H
#define RAMSES_GEOMETRYBINDINGIMPL_H

// internal
#include "SceneObjectImpl.h"
#include "ArrayResourceImpl.h"

// ramses framework
#include "SceneAPI/EDataType.h"

namespace ramses_internal
{
    class IScene;
}

namespace ramses
{
    class ResourceImpl;
    class EffectImpl;
    class AttributeInput;
    class ArrayBufferImpl;
    class ArrayResourceImpl;

    class GeometryBindingImpl final : public SceneObjectImpl
    {
    public:
        GeometryBindingImpl(SceneImpl& scene, const char* name);
        ~GeometryBindingImpl() override;

        void             initializeFrameworkData(const EffectImpl& effect);
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        status_t validate() const override;

        status_t setInputBuffer(const EffectInputImpl& input, const ArrayResourceImpl& bufferResource, uint32_t instancingDivisor, uint16_t offset, uint16_t stride);
        status_t setInputBuffer(const EffectInputImpl& input, const ArrayBufferImpl& dataBuffer, uint32_t instancingDivisor, uint16_t offset, uint16_t stride);
        status_t setIndices(const ArrayResourceImpl& arrayResource);
        status_t setIndices(const ArrayBufferImpl& dataBuffer);

        ramses_internal::ResourceContentHash getEffectHash() const;
        ramses_internal::DataLayoutHandle    getAttributeDataLayout() const;
        ramses_internal::DataInstanceHandle  getAttributeDataInstance() const;
        uint32_t                             getIndicesCount() const;

        const Effect& getEffect() const;

        static const uint32_t IndicesDataFieldIndex = 0u;

    private:
        void createDataLayout();

        status_t validateEffect() const;
        status_t validateAttribute() const;
        status_t validateResource(ramses_internal::ResourceContentHash resourceHash) const;
        status_t validateDataBuffer(ramses_internal::DataBufferHandle dataBuffer, ramses_internal::EDataType fieldDataType) const;
        ArrayBufferImpl* findDataBuffer(ramses_internal::DataBufferHandle dataBufferHandle) const;

        static bool dataTypeMatchesInputType(ramses_internal::EDataType resourceType, ramses_internal::EDataType inputDataType);

        const EffectImpl*                       m_effectImpl;
        ramses_internal::DataLayoutHandle       m_attributeLayout;
        ramses_internal::DataInstanceHandle     m_attributeInstance;
        uint32_t                                m_indicesCount;
    };
}

#endif
