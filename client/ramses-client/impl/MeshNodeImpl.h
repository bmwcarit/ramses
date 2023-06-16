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

#include <string_view>

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

    class MeshNodeImpl final : public NodeImpl
    {
    public:
        MeshNodeImpl(SceneImpl& scene, std::string_view nodeName);
        ~MeshNodeImpl() override;

        void             initializeFrameworkData();
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        status_t validate() const override;

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
