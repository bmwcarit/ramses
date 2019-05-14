//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderGroupImpl.h"

#include "ramses-client-api/MeshNode.h"

#include "SerializationContext.h"
#include "MeshNodeImpl.h"

#include "SceneObjectImpl.h"
#include "SceneImpl.h"
#include "Scene/ClientScene.h"
#include "SceneAPI/RenderGroup.h"
#include "SceneAPI/RenderGroupUtils.h"


namespace ramses
{
    RenderGroupImpl::RenderGroupImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_RenderGroup, name)
    {
    }

    RenderGroupImpl::~RenderGroupImpl()
    {
    }

    template <typename OBJECT>
    void serializeObjects(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext, const std::vector<const OBJECT*>& objects)
    {
        outStream << static_cast<uint32_t>(objects.size());
        for(const auto& object : objects)
        {
            assert(0 != object);
            outStream << serializationContext.getIDForObject(object);
        }
    }

    status_t RenderGroupImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_renderGroupHandle;

        serializeObjects(outStream, serializationContext, m_meshes);
        serializeObjects(outStream, serializationContext, m_renderGroups);

        return StatusOK;
    }

    template <typename OBJECT>
    void deserializeObjects(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, std::vector<const OBJECT*>& objects)
    {
        UNUSED(serializationContext)
        uint32_t numberOfObjects = 0;
        inStream >> numberOfObjects;
        assert(objects.empty());
        objects.reserve(numberOfObjects);

        for (uint32_t i = 0; i < numberOfObjects; ++i)
        {
            OBJECT* object = 0;
            serializationContext.ReadDependentPointerAndStoreAsID(inStream, object);
            objects.push_back(object);
        }
    }

    status_t RenderGroupImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_renderGroupHandle;

        deserializeObjects(inStream, serializationContext, m_meshes);
        deserializeObjects(inStream, serializationContext, m_renderGroups);

        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t RenderGroupImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        for (uint32_t i = 0; i < m_meshes.size(); ++i)
        {
            MeshNodeImpl* mesh = const_cast<MeshNodeImpl*>(m_meshes[i]);
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(mesh);
            m_meshes[i] = mesh;
        }

        for (uint32_t i = 0; i < m_renderGroups.size(); ++i)
        {
            RenderGroupImpl* renderGroup = const_cast<RenderGroupImpl*>(m_renderGroups[i]);
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(renderGroup);
            m_renderGroups[i] = renderGroup;
        }

        return StatusOK;
    }

    status_t RenderGroupImpl::validate(uint32_t indent) const
    {
        status_t status = SceneObjectImpl::validate(indent);
        indent += IndentationStep;

        if (m_meshes.size() == 0u)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "rendergroup does not contain any meshes");
            status = getValidationErrorStatus();
        }

        validateElements(indent, status, m_meshes);
        validateElements(indent, status, m_renderGroups);

        return status;
    }

    void RenderGroupImpl::initializeFrameworkData()
    {
        m_renderGroupHandle = getIScene().allocateRenderGroup();
    }

    void RenderGroupImpl::deinitializeFrameworkData()
    {
        assert(m_renderGroupHandle.isValid());
        getIScene().releaseRenderGroup(m_renderGroupHandle);
        m_renderGroupHandle = ramses_internal::RenderGroupHandle::Invalid();
    }

    bool RenderGroupImpl::contains(const MeshNodeImpl& meshImpl) const
    {
        // const cast just to get ptr for query
        return ramses_internal::contains_c(m_meshes, &meshImpl);
    }

    const MeshNodeImplVector& RenderGroupImpl::getAllMeshes() const
    {
        return m_meshes;
    }

    status_t RenderGroupImpl::getMeshNodeOrder(const MeshNodeImpl& mesh, int32_t& orderWithinGroup) const
    {
        if (!contains(mesh))
        {
            return addErrorEntry("RenderGroup::getMeshNodeOrder failed - mesh not contained in RenderGroup");
        }

        const auto& internalRenderGroup = getIScene().getRenderGroup(m_renderGroupHandle);
        const auto renderableEntryIt = ramses_internal::RenderGroupUtils::FindRenderableEntry(mesh.getRenderableHandle(), internalRenderGroup);
        if (renderableEntryIt == internalRenderGroup.renderables.cend())
        {
            assert(false);
            return addErrorEntry("RenderGroup::getMeshNodeOrder failed - fatal, mesh not found in internal render group");
        }

        orderWithinGroup = renderableEntryIt->order;
        return StatusOK;
    }

    status_t RenderGroupImpl::addMeshNode(const MeshNodeImpl& meshImpl, int32_t orderWithinGroup)
    {
        if (!isFromTheSameSceneAs(meshImpl))
        {
            return addErrorEntry("RenderGroup::addMeshNode failed - meshNode is not from the same scene as this RenderGroup.");
        }

        if (ramses_internal::contains_c(m_meshes, &meshImpl))
        {
            remove(meshImpl);
        }

        const ramses_internal::RenderableHandle renderableToAdd = meshImpl.getRenderableHandle();
        getIScene().addRenderableToRenderGroup(m_renderGroupHandle, renderableToAdd, orderWithinGroup);
        m_meshes.push_back(&meshImpl);

        return StatusOK;
    }

    status_t RenderGroupImpl::remove(const MeshNodeImpl& mesh)
    {
        MeshNodeImplVector::iterator iter = ramses_internal::find_c(m_meshes, &mesh);
        if (iter == m_meshes.end())
        {
            return addErrorEntry("RenderGroup::remove failed - could not remove MeshNode from RenderGroup because it was not contained");
        }

        removeInternal(iter);

        return StatusOK;
    }

    void RenderGroupImpl::removeIfContained(const MeshNodeImpl& mesh)
    {
        MeshNodeImplVector::iterator iter = ramses_internal::find_c(m_meshes, &mesh);
        if (iter != m_meshes.end())
        {
            removeInternal(iter);
        }
    }

    status_t RenderGroupImpl::addRenderGroup(const RenderGroupImpl& renderGroupImpl, int32_t orderWithinGroup)
    {
        if (!isFromTheSameSceneAs(renderGroupImpl))
        {
            return addErrorEntry("RenderGroup::addRenderGroup failed - renderGroup is not from the same scene as this RenderGroup.");
        }

        if (ramses_internal::contains_c(m_renderGroups, &renderGroupImpl))
        {
            remove(renderGroupImpl);
        }

        const ramses_internal::RenderGroupHandle renderGroupToAdd = renderGroupImpl.getRenderGroupHandle();
        getIScene().addRenderGroupToRenderGroup(m_renderGroupHandle, renderGroupToAdd, orderWithinGroup);
        m_renderGroups.push_back(&renderGroupImpl);

        return StatusOK;
    }

    status_t RenderGroupImpl::remove(const RenderGroupImpl& renderGroup)
    {
        RenderGroupImplVector::iterator iter = ramses_internal::find_c(m_renderGroups, &renderGroup);
        if (iter == m_renderGroups.end())
        {
            return addErrorEntry("RenderGroup::removeRenderGroup failed - could not remove render group from RenderGroup because it was not contained");
        }

        removeInternal(iter);

        return StatusOK;
    }

    void RenderGroupImpl::removeIfContained(const RenderGroupImpl& renderGroup)
    {
        RenderGroupImplVector::iterator iter = ramses_internal::find_c(m_renderGroups, &renderGroup);
        if (iter != m_renderGroups.end())
        {
            removeInternal(iter);
        }
    }

    bool RenderGroupImpl::contains(const RenderGroupImpl& renderGroup) const
    {
        return ramses_internal::contains_c(m_renderGroups, &renderGroup);
    }

    status_t RenderGroupImpl::getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinGroup) const
    {
        if (!contains(renderGroup))
        {
            return addErrorEntry("RenderGroup::getRenderGroupOrder failed - render group not contained in RenderGroup");
        }

        const auto& internalRenderGroup = getIScene().getRenderGroup(m_renderGroupHandle);
        const auto renderGroupEntryIt = ramses_internal::RenderGroupUtils::FindRenderGroupEntry(renderGroup.getRenderGroupHandle(), internalRenderGroup);
        if (renderGroupEntryIt == internalRenderGroup.renderGroups.cend())
        {
            assert(false);
            return addErrorEntry("RenderGroup::getRenderGroupOrder failed - fatal, render group not found in internal render group");
        }

        orderWithinGroup = renderGroupEntryIt->order;
        return StatusOK;
    }

    const RenderGroupImplVector& RenderGroupImpl::getAllRenderGroups() const
    {
        return m_renderGroups;
    }

    status_t RenderGroupImpl::removeAllRenderables()
    {
        while (!m_meshes.empty())
        {
            CHECK_RETURN_ERR(remove(*m_meshes.front()));
        }

        return StatusOK;
    }

    status_t RenderGroupImpl::removeAllRenderGroups()
    {
        while (!m_renderGroups.empty())
        {
            CHECK_RETURN_ERR(remove(*m_renderGroups.front()));
        }

        return StatusOK;
    }

    ramses_internal::RenderGroupHandle RenderGroupImpl::getRenderGroupHandle() const
    {
        return m_renderGroupHandle;
    }

    void RenderGroupImpl::removeInternal(MeshNodeImplVector::iterator iter)
    {
        const ramses_internal::RenderableHandle renderableToRemove = (*iter)->getRenderableHandle();
        getIScene().removeRenderableFromRenderGroup(m_renderGroupHandle, renderableToRemove);
        m_meshes.erase(iter);
    }

    void RenderGroupImpl::removeInternal(RenderGroupImplVector::iterator iter)
    {
        const ramses_internal::RenderGroupHandle renderGroupToRemove = (*iter)->getRenderGroupHandle();
        getIScene().removeRenderGroupFromRenderGroup(m_renderGroupHandle, renderGroupToRemove);
        m_renderGroups.erase(iter);
    }

    template <typename ELEMENT>
    void RenderGroupImpl::validateElements(uint32_t& indent, status_t& status, const std::vector<const ELEMENT*>& elements) const
    {
        for(const auto& element : elements)
        {
            if (addValidationOfDependentObject(indent, *element) != StatusOK)
            {
                status = getValidationErrorStatus();
            }
        }
    }
}
