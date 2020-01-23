//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MESHNODEIMPL_H
#define RAMSES_MESHNODEIMPL_H

// internal
#include "NodeImpl.h"

namespace ramses_internal
{
    class ClientApplicationLogic;
}

namespace ramses
{
    class Appearance;
    class GeometryBinding;
    class GeometryBindingImpl;
    class AppearanceImpl;
    class UInt16Array;

    class MeshNodeImpl final : public NodeImpl
    {
    public:
        MeshNodeImpl(SceneImpl& scene, const char* nodeName);
        virtual ~MeshNodeImpl();

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent) const override;

        status_t setAppearance(AppearanceImpl& appearanceImpl);
        status_t setGeometryBinding(GeometryBindingImpl& geometryImpl);
        status_t removeAppearanceAndGeometry();
        status_t setStartIndex(uint32_t startIndex);
        uint32_t getStartIndex() const;
        status_t setIndexCount(uint32_t indexCount);
        uint32_t getIndexCount() const;
        status_t setFlattenedVisibility(EVisibilityMode mode);
        EVisibilityMode getFlattenedVisibility() const;
        status_t setInstanceCount(uint32_t instanceCount);
        uint32_t getInstanceCount() const;
        status_t setStartVertex(uint32_t startVertex);
        uint32_t getStartVertex() const;

        ramses_internal::RenderableHandle   getRenderableHandle() const;

        const AppearanceImpl*  getAppearanceImpl() const;
        const Appearance*      getAppearance() const;
        Appearance*            getAppearance();

        const GeometryBindingImpl* getGeometryBindingImpl() const;
        const GeometryBinding* getGeometryBinding() const;
        GeometryBinding*       getGeometryBinding();

    private:
        static bool AreGeometryAndAppearanceCompatible(const GeometryBindingImpl& geometry, const AppearanceImpl& appearance);

        ramses_internal::RenderableHandle       m_renderableHandle;

        const AppearanceImpl*      m_appearanceImpl;
        const GeometryBindingImpl* m_geometryImpl;
    };
}

#endif
