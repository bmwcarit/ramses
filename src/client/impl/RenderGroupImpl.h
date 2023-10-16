//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneObjectImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <vector>
#include <string_view>

namespace ramses::internal
{
    class MeshNodeImpl;
    class RenderGroupImpl;

    using MeshNodeImplVector = std::vector<const MeshNodeImpl *>;
    using RenderGroupImplVector = std::vector<const RenderGroupImpl *>;

    class RenderGroupImpl final : public SceneObjectImpl
    {
    public:
        RenderGroupImpl(SceneImpl& scene, std::string_view name);
        ~RenderGroupImpl() override;

        void initializeFrameworkData();
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        bool               addMeshNode(const MeshNodeImpl& mesh, int32_t orderWithinGroup);
        bool               remove(const MeshNodeImpl& mesh);
        void               removeIfContained(const MeshNodeImpl& mesh);
        [[nodiscard]] bool contains(const MeshNodeImpl& mesh) const;
        bool               getMeshNodeOrder(const MeshNodeImpl& mesh, int32_t& orderWithinGroup) const;
        [[nodiscard]] const MeshNodeImplVector& getAllMeshes() const;

        bool               addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinGroup);
        bool               remove(const RenderGroupImpl& renderGroup);
        void               removeIfContained(const RenderGroupImpl& renderGroup);
        [[nodiscard]] bool contains(const RenderGroupImpl& renderGroup) const;
        bool               getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinGroup) const;
        [[nodiscard]] const RenderGroupImplVector&    getAllRenderGroups() const;

        bool removeAllRenderables();
        bool removeAllRenderGroups();

        [[nodiscard]] ramses::internal::RenderGroupHandle getRenderGroupHandle() const;

    private:
        void removeInternal(MeshNodeImplVector::iterator iter);
        void removeInternal(RenderGroupImplVector::iterator iter);

        ramses::internal::RenderGroupHandle  m_renderGroupHandle;

        MeshNodeImplVector m_meshes;
        RenderGroupImplVector m_renderGroups;
    };
}
