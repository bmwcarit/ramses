//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERGROUPIMPL_H
#define RAMSES_RENDERGROUPIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"

#include <vector>
#include <string_view>

namespace ramses
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

        void             initializeFrameworkData();
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        status_t validate() const override;

        status_t                  addMeshNode(const MeshNodeImpl& mesh, int32_t orderWithinGroup);
        status_t                  remove(const MeshNodeImpl& mesh);
        void                      removeIfContained(const MeshNodeImpl& mesh);
        bool                      contains(const MeshNodeImpl& mesh) const;
        status_t                  getMeshNodeOrder(const MeshNodeImpl& mesh, int32_t& orderWithinGroup) const;
        const MeshNodeImplVector& getAllMeshes() const;

        status_t                        addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinGroup);
        status_t                        remove(const RenderGroupImpl& renderGroup);
        void                            removeIfContained(const RenderGroupImpl& renderGroup);
        bool                            contains(const RenderGroupImpl& renderGroup) const;
        status_t                        getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinGroup) const;
        const RenderGroupImplVector&    getAllRenderGroups() const;

        status_t                  removeAllRenderables();
        status_t                  removeAllRenderGroups();

        ramses_internal::RenderGroupHandle getRenderGroupHandle() const;

    private:
        void removeInternal(MeshNodeImplVector::iterator iter);
        void removeInternal(RenderGroupImplVector::iterator iter);

        ramses_internal::RenderGroupHandle  m_renderGroupHandle;

        MeshNodeImplVector m_meshes;
        RenderGroupImplVector m_renderGroups;
    };
}

#endif
