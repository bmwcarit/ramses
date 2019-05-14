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
#include "Collections/Vector.h"

namespace ramses
{
    class MeshNodeImpl;
    class RenderGroupImpl;

    typedef std::vector<const MeshNodeImpl*>  MeshNodeImplVector;
    typedef std::vector<const RenderGroupImpl*>    RenderGroupImplVector;

    class RenderGroupImpl final : public SceneObjectImpl
    {
    public:
        RenderGroupImpl(SceneImpl& scene, const char* name);
        virtual ~RenderGroupImpl();

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent) const override;

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

        template <typename ELEMENT>
        void validateElements(uint32_t& indent, status_t& status, const std::vector<const ELEMENT*>& elements) const;

        ramses_internal::RenderGroupHandle  m_renderGroupHandle;

        MeshNodeImplVector m_meshes;
        RenderGroupImplVector m_renderGroups;
    };
}

#endif
