//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RenderGroupImpl.h"

#include "ramses/client/MeshNode.h"

#include "impl/SerializationContext.h"
#include "impl/MeshNodeImpl.h"
#include "impl/SceneObjectImpl.h"
#include "impl/SceneImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneAPI/RenderGroup.h"
#include "internal/SceneGraph/SceneAPI/RenderGroupUtils.h"


namespace ramses::internal
{
    RenderGroupImpl::RenderGroupImpl(SceneImpl& scene, std::string_view name)
        : SceneObjectImpl(scene, ERamsesObjectType::RenderGroup, name)
    {
    }

    RenderGroupImpl::~RenderGroupImpl() = default;

    template <typename OBJECT>
    void serializeObjects(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext, const std::vector<const OBJECT*>& objects)
    {
        outStream << static_cast<uint32_t>(objects.size());
        for(const auto& object : objects)
        {
            assert(nullptr != object);
            outStream << serializationContext.getIDForObject(object);
        }
    }

    bool RenderGroupImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_renderGroupHandle;

        serializeObjects(outStream, serializationContext, m_meshes);
        serializeObjects(outStream, serializationContext, m_renderGroups);

        return true;
    }

    template <typename OBJECT>
    void deserializeObjects(ramses::internal::IInputStream& inStream, [[maybe_unused]] DeserializationContext& serializationContext, std::vector<const OBJECT*>& objects)
    {
        uint32_t numberOfObjects = 0;
        inStream >> numberOfObjects;
        assert(objects.empty());
        objects.reserve(numberOfObjects);

        for (uint32_t i = 0; i < numberOfObjects; ++i)
        {
            OBJECT* object = nullptr;
            serializationContext.ReadDependentPointerAndStoreAsID(inStream, object);
            objects.push_back(object);
        }
    }

    bool RenderGroupImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_renderGroupHandle;

        deserializeObjects(inStream, serializationContext, m_meshes);
        deserializeObjects(inStream, serializationContext, m_renderGroups);

        serializationContext.addForDependencyResolve(this);

        return true;
    }

    bool RenderGroupImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        for (auto& mesh : m_meshes)
        {
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(mesh);
        }

        for (auto& group : m_renderGroups)
        {
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(group);
        }

        return true;
    }

    void RenderGroupImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        if (m_meshes.empty() && m_renderGroups.empty())
            report.add(EIssueType::Warning, "rendergroup does not contain any meshes", &getRamsesObject());

        for (const auto& element : m_meshes)
            report.addDependentObject(*this, *element);

        for (const auto& element : m_renderGroups)
            report.addDependentObject(*this, *element);
    }

    void RenderGroupImpl::initializeFrameworkData()
    {
        m_renderGroupHandle = getIScene().allocateRenderGroup(0, 0, {});
    }

    void RenderGroupImpl::deinitializeFrameworkData()
    {
        assert(m_renderGroupHandle.isValid());
        getIScene().releaseRenderGroup(m_renderGroupHandle);
        m_renderGroupHandle = ramses::internal::RenderGroupHandle::Invalid();
    }

    bool RenderGroupImpl::contains(const MeshNodeImpl& meshImpl) const
    {
        // const cast just to get ptr for query
        return ramses::internal::contains_c(m_meshes, &meshImpl);
    }

    const MeshNodeImplVector& RenderGroupImpl::getAllMeshes() const
    {
        return m_meshes;
    }

    bool RenderGroupImpl::getMeshNodeOrder(const MeshNodeImpl& mesh, int32_t& orderWithinGroup) const
    {
        if (!contains(mesh))
        {
            getErrorReporting().set("RenderGroup::getMeshNodeOrder failed - mesh not contained in RenderGroup", *this);
            return false;
        }

        const auto& internalRenderGroup = getIScene().getRenderGroup(m_renderGroupHandle);
        const auto renderableEntryIt = ramses::internal::RenderGroupUtils::FindRenderableEntry(mesh.getRenderableHandle(), internalRenderGroup);
        if (renderableEntryIt == internalRenderGroup.renderables.cend())
        {
            assert(false);
            getErrorReporting().set("RenderGroup::getMeshNodeOrder failed - fatal, mesh not found in internal render group", *this);
            return false;
        }

        orderWithinGroup = renderableEntryIt->order;
        return true;
    }

    bool RenderGroupImpl::addMeshNode(const MeshNodeImpl& meshImpl, int32_t orderWithinGroup)
    {
        if (!isFromTheSameSceneAs(meshImpl))
        {
            getErrorReporting().set("RenderGroup::addMeshNode failed - meshNode is not from the same scene as this RenderGroup.", *this);
            return false;
        }

        if (ramses::internal::contains_c(m_meshes, &meshImpl))
        {
            remove(meshImpl);
        }

        const ramses::internal::RenderableHandle renderableToAdd = meshImpl.getRenderableHandle();
        getIScene().addRenderableToRenderGroup(m_renderGroupHandle, renderableToAdd, orderWithinGroup);
        m_meshes.push_back(&meshImpl);

        return true;
    }

    bool RenderGroupImpl::remove(const MeshNodeImpl& mesh)
    {
        auto iter = ramses::internal::find_c(m_meshes, &mesh);
        if (iter == m_meshes.end())
        {
            getErrorReporting().set("RenderGroup::remove failed - could not remove MeshNode from RenderGroup because it was not contained", *this);
            return false;
        }

        removeInternal(iter);

        return true;
    }

    void RenderGroupImpl::removeIfContained(const MeshNodeImpl& mesh)
    {
        auto iter = ramses::internal::find_c(m_meshes, &mesh);
        if (iter != m_meshes.end())
        {
            removeInternal(iter);
        }
    }

    bool RenderGroupImpl::addRenderGroup(const RenderGroupImpl& renderGroupImpl, int32_t orderWithinGroup)
    {
        if (!isFromTheSameSceneAs(renderGroupImpl))
        {
            getErrorReporting().set("RenderGroup::addRenderGroup failed - renderGroup is not from the same scene as this RenderGroup.", *this);
            return false;
        }

        if (ramses::internal::contains_c(m_renderGroups, &renderGroupImpl))
        {
            remove(renderGroupImpl);
        }

        const ramses::internal::RenderGroupHandle renderGroupToAdd = renderGroupImpl.getRenderGroupHandle();
        getIScene().addRenderGroupToRenderGroup(m_renderGroupHandle, renderGroupToAdd, orderWithinGroup);
        m_renderGroups.push_back(&renderGroupImpl);

        return true;
    }

    bool RenderGroupImpl::remove(const RenderGroupImpl& renderGroup)
    {
        auto iter = ramses::internal::find_c(m_renderGroups, &renderGroup);
        if (iter == m_renderGroups.end())
        {
            getErrorReporting().set("RenderGroup::removeRenderGroup failed - could not remove render group from RenderGroup because it was not contained", *this);
            return false;
        }

        removeInternal(iter);

        return true;
    }

    void RenderGroupImpl::removeIfContained(const RenderGroupImpl& renderGroup)
    {
        auto iter = ramses::internal::find_c(m_renderGroups, &renderGroup);
        if (iter != m_renderGroups.end())
        {
            removeInternal(iter);
        }
    }

    bool RenderGroupImpl::contains(const RenderGroupImpl& renderGroup) const
    {
        return ramses::internal::contains_c(m_renderGroups, &renderGroup);
    }

    bool RenderGroupImpl::getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinGroup) const
    {
        if (!contains(renderGroup))
        {
            getErrorReporting().set("RenderGroup::getRenderGroupOrder failed - render group not contained in RenderGroup", *this);
            return false;
        }

        const auto& internalRenderGroup = getIScene().getRenderGroup(m_renderGroupHandle);
        const auto renderGroupEntryIt = ramses::internal::RenderGroupUtils::FindRenderGroupEntry(renderGroup.getRenderGroupHandle(), internalRenderGroup);
        if (renderGroupEntryIt == internalRenderGroup.renderGroups.cend())
        {
            getErrorReporting().set("RenderGroup::getRenderGroupOrder failed - fatal, render group not found in internal render group", *this);
            assert(false);
            return false;
        }

        orderWithinGroup = renderGroupEntryIt->order;
        return true;
    }

    const RenderGroupImplVector& RenderGroupImpl::getAllRenderGroups() const
    {
        return m_renderGroups;
    }

    bool RenderGroupImpl::removeAllRenderables()
    {
        while (!m_meshes.empty())
        {
            if (!remove(*m_meshes.front()))
                return false;
        }

        return true;
    }

    bool RenderGroupImpl::removeAllRenderGroups()
    {
        while (!m_renderGroups.empty())
        {
            if (!remove(*m_renderGroups.front()))
                return false;
        }

        return true;
    }

    ramses::internal::RenderGroupHandle RenderGroupImpl::getRenderGroupHandle() const
    {
        return m_renderGroupHandle;
    }

    void RenderGroupImpl::removeInternal(MeshNodeImplVector::iterator iter)
    {
        const ramses::internal::RenderableHandle renderableToRemove = (*iter)->getRenderableHandle();
        getIScene().removeRenderableFromRenderGroup(m_renderGroupHandle, renderableToRemove);
        m_meshes.erase(iter);
    }

    void RenderGroupImpl::removeInternal(RenderGroupImplVector::iterator iter)
    {
        const ramses::internal::RenderGroupHandle renderGroupToRemove = (*iter)->getRenderGroupHandle();
        getIScene().removeRenderGroupFromRenderGroup(m_renderGroupHandle, renderGroupToRemove);
        m_renderGroups.erase(iter);
    }
}
