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
    class UInt16Array;
    class UInt32Array;
    class EffectImpl;
    class AttributeInput;
    class IndexDataBufferImpl;
    class VertexDataBufferImpl;
    class DataBufferImpl;

    class GeometryBindingImpl final : public SceneObjectImpl
    {
    public:
        GeometryBindingImpl(SceneImpl& scene, const char* name);
        virtual ~GeometryBindingImpl();

        void             initializeFrameworkData(const EffectImpl& effect);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        status_t setInputBuffer(const EffectInputImpl& input, const ResourceImpl& bufferResource, uint32_t instancingDivisor = 0);
        status_t setInputBuffer(const EffectInputImpl& input, const VertexDataBufferImpl& dataBuffer, uint32_t instancingDivisor = 0);
        status_t setIndices(const ArrayResourceImpl& arrayResource);
        status_t setIndices(const IndexDataBufferImpl& dataBuffer);

        ramses_internal::ResourceContentHash getEffectHash() const;
        ramses_internal::DataLayoutHandle    getAttributeDataLayout() const;
        ramses_internal::DataInstanceHandle  getAttributeDataInstance() const;
        uint32_t                             getIndicesCount() const;

        const Effect& getEffect() const;

        static const uint32_t IndicesDataFieldIndex = 0u;

    private:
        void createDataLayout();

        status_t validateEffect(uint32_t indent, StatusObjectSet& visitedObjects) const;
        status_t validateAttribute(uint32_t indent, StatusObjectSet& visitedObjects) const;
        status_t validateResource(uint32_t indent, ramses_internal::ResourceContentHash resourceHash, StatusObjectSet& visitedObjects) const;
        status_t validateDataBuffer(uint32_t indent, ramses_internal::DataBufferHandle dataBuffer, ramses_internal::EDataType fieldDataType, StatusObjectSet& visitedObjects) const;
        DataBufferImpl* findDataBuffer(ramses_internal::DataBufferHandle dataBufferHandle) const;

        static bool resourceDataTypeMatchesInputType(ERamsesObjectType resourceType, ramses_internal::EDataType inputDataType);
        static bool dataBufferDataTypeMatchesInputType(ramses_internal::EDataType resourceType, ramses_internal::EDataType inputDataType);

        const EffectImpl*                       m_effectImpl;
        ramses_internal::DataLayoutHandle       m_attributeLayout;
        ramses_internal::DataInstanceHandle     m_attributeInstance;
        uint32_t                                m_indicesCount;
    };
}

#endif
