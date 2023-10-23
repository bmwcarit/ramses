//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// internal
#include "impl/NodeImpl.h"

#include <string_view>

namespace ramses::internal
{
    class ClientApplicationLogic;
}

namespace ramses
{
    class Appearance;
    class Geometry;
}

namespace ramses::internal
{
    class GeometryImpl;
    class AppearanceImpl;

    class MeshNodeImpl final : public NodeImpl
    {
    public:
        MeshNodeImpl(SceneImpl& scene, std::string_view nodeName);
        ~MeshNodeImpl() override;

        void initializeFrameworkData();
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        bool setAppearance(AppearanceImpl& appearanceImpl);
        bool setGeometry(GeometryImpl& geometryImpl);
        bool removeAppearanceAndGeometry();
        bool setStartIndex(uint32_t startIndex);
        [[nodiscard]] uint32_t getStartIndex() const;
        bool setIndexCount(uint32_t indexCount);
        [[nodiscard]] uint32_t getIndexCount() const;
        bool setFlattenedVisibility(EVisibilityMode mode);
        [[nodiscard]] EVisibilityMode getFlattenedVisibility() const;
        bool setInstanceCount(uint32_t instanceCount);
        [[nodiscard]] uint32_t getInstanceCount() const;
        bool setStartVertex(uint32_t startVertex);
        [[nodiscard]] uint32_t getStartVertex() const;

        [[nodiscard]] ramses::internal::RenderableHandle   getRenderableHandle() const;

        [[nodiscard]] const AppearanceImpl*  getAppearanceImpl() const;
        [[nodiscard]] const Appearance*      getAppearance() const;
        [[nodiscard]] Appearance*            getAppearance();

        [[nodiscard]] const GeometryImpl* getGeometryImpl() const;
        [[nodiscard]] const Geometry* getGeometry() const;
        [[nodiscard]] Geometry*       getGeometry();

    private:
        static bool AreGeometryAndAppearanceCompatible(const GeometryImpl& geometry, const AppearanceImpl& appearance);

        ramses::internal::RenderableHandle       m_renderableHandle;

        const AppearanceImpl*      m_appearanceImpl;
        const GeometryImpl* m_geometryImpl;
    };
}
