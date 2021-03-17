//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "MeshNodeImpl.h"

// API
#include "ramses-client-api/GeometryBinding.h"

// impls
#include "AppearanceImpl.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "AppearanceImpl.h"
#include "GeometryBindingImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "VisibilityModeUtils.h"

// internal
#include "Resource/IResource.h"
#include "SerializationContext.h"
#include "Scene/ClientScene.h"

namespace ramses
{
    MeshNodeImpl::MeshNodeImpl(SceneImpl& scene, const char* nodeName)
        : NodeImpl(scene, ERamsesObjectType_MeshNode, nodeName)
        , m_appearanceImpl(nullptr)
        , m_geometryImpl(nullptr)
    {
    }

    MeshNodeImpl::~MeshNodeImpl()
    {
    }

    status_t MeshNodeImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(NodeImpl::serialize(outStream, serializationContext));

        outStream << m_renderableHandle;
        if (m_appearanceImpl != nullptr)
        {
            outStream << serializationContext.getIDForObject(m_appearanceImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }
        if (m_geometryImpl != nullptr)
        {
            outStream << serializationContext.getIDForObject(m_geometryImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }

        return StatusOK;
    }

    status_t MeshNodeImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(NodeImpl::deserialize(inStream, serializationContext));

        inStream >> m_renderableHandle;

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_appearanceImpl);
        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_geometryImpl);
        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t MeshNodeImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(NodeImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_appearanceImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_geometryImpl);

        return StatusOK;
    }

    status_t MeshNodeImpl::validate() const
    {
        status_t status = NodeImpl::validate();
        if (nullptr == m_appearanceImpl)
            status = addValidationMessage(EValidationSeverity_Error, "meshnode does not have an appearance set");
        else
            status = std::max(status, addValidationOfDependentObject(*m_appearanceImpl));

        if (nullptr == m_geometryImpl)
            status = addValidationMessage(EValidationSeverity_Error, "meshnode does not have a geometryBinding set");
        else
        {
            status = std::max(status, addValidationOfDependentObject(*m_geometryImpl));

            const bool hasIndexArray = m_geometryImpl->getIndicesCount() > 0;
            if (hasIndexArray && (m_geometryImpl->getIndicesCount() < getStartIndex() + getIndexCount()))
                status = addValidationMessage(EValidationSeverity_Error, "startIndex + indexCount exceeds indices of indexarray");

            if (getIndexCount() == 0)
                status = addValidationMessage(EValidationSeverity_Error, "indexCount must be greater 0");
        }
        return status;
    }

    void MeshNodeImpl::initializeFrameworkData()
    {
        NodeImpl::initializeFrameworkData();

        m_renderableHandle = getIScene().allocateRenderable(getNodeHandle(), ramses_internal::RenderableHandle::Invalid());
    }

    void MeshNodeImpl::deinitializeFrameworkData()
    {
        assert(m_renderableHandle.isValid());
        getIScene().releaseRenderable(m_renderableHandle);

        NodeImpl::deinitializeFrameworkData();
    }

    status_t MeshNodeImpl::setAppearance(AppearanceImpl& appearance)
    {
        if (!isFromTheSameSceneAs(appearance))
        {
            return addErrorEntry("MeshNode::setAppearance failed - appearance is not from the same scene as this MeshNode.");
        }

        if (m_appearanceImpl == &appearance)
        {
            return StatusOK;
        }

        if (m_geometryImpl != nullptr && !AreGeometryAndAppearanceCompatible(*m_geometryImpl, appearance))
        {
            return addErrorEntry("MeshNode::setAppearance failed - previously assigned geometry does not provide all vertex attributes required by the appearance!");
        }

        m_appearanceImpl = &appearance;

        getIScene().setRenderableUniformsDataInstanceAndState(m_renderableHandle, appearance.getUniformDataInstance(), appearance.getRenderStateHandle());

        return StatusOK;
    }

    status_t MeshNodeImpl::setGeometryBinding(GeometryBindingImpl& geometry)
    {
        if (!isFromTheSameSceneAs(geometry))
        {
            return addErrorEntry("MeshNode::setGeometryBinding failed - geometry is not from the same scene as this MeshNode.");
        }

        if (m_geometryImpl == &geometry)
        {
            return StatusOK;
        }

        if (m_appearanceImpl != nullptr && !AreGeometryAndAppearanceCompatible(geometry, *m_appearanceImpl))
        {
            return addErrorEntry("MeshNode::setGeometry failed - the geometry does not provide all vertex attributes required by previously assigned appearance!");
        }

        m_geometryImpl = &geometry;

        const uint32_t numberOfIndicesFromGeometryBinding = geometry.getIndicesCount();
        const bool geometryBindingProvidesIndexArray = numberOfIndicesFromGeometryBinding > 0;
        if (geometryBindingProvidesIndexArray)
        {
            setStartIndex(0u);
            setIndexCount(numberOfIndicesFromGeometryBinding);
        }
        getIScene().setRenderableDataInstance(m_renderableHandle, ramses_internal::ERenderableDataSlotType_Geometry, geometry.getAttributeDataInstance());

        return StatusOK;
    }

    status_t MeshNodeImpl::removeAppearanceAndGeometry()
    {
        m_appearanceImpl = nullptr;

        getIScene().setRenderableUniformsDataInstanceAndState(m_renderableHandle, ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::RenderStateHandle::Invalid());

        m_geometryImpl = nullptr;

        getIScene().setRenderableDataInstance(m_renderableHandle, ramses_internal::ERenderableDataSlotType_Geometry, ramses_internal::DataInstanceHandle::Invalid());
        return setIndexCount(0u);
    }

    status_t MeshNodeImpl::setStartIndex(uint32_t startIndex)
    {
        getIScene().setRenderableStartIndex(m_renderableHandle, startIndex);
        return StatusOK;
    }

    status_t MeshNodeImpl::setIndexCount(uint32_t indexCount)
    {
        getIScene().setRenderableIndexCount(m_renderableHandle, indexCount);
        return StatusOK;
    }

    status_t MeshNodeImpl::setFlattenedVisibility(EVisibilityMode mode)
    {
        getIScene().setRenderableVisibility(m_renderableHandle, VisibilityModeUtils::ConvertToLL(mode));
        return StatusOK;
    }

    status_t MeshNodeImpl::setInstanceCount(uint32_t instanceCount)
    {
        if (instanceCount == 0)
        {
            return addErrorEntry("MeshNode::setInstanceCount failed: instance count must not be 0!");
        }

        getIScene().setRenderableInstanceCount(m_renderableHandle, instanceCount);
        return StatusOK;
    }

    ramses::status_t MeshNodeImpl::setStartVertex(uint32_t startVertex)
    {
        getIScene().setRenderableStartVertex(m_renderableHandle, startVertex);
        return StatusOK;
    }

    ramses_internal::RenderableHandle MeshNodeImpl::getRenderableHandle() const
    {
        return m_renderableHandle;
    }

    const AppearanceImpl* MeshNodeImpl::getAppearanceImpl() const
    {
        return m_appearanceImpl;
    }

    const Appearance* MeshNodeImpl::getAppearance() const
    {
        if (m_appearanceImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo<Appearance>(m_appearanceImpl->getRamsesObject());
        }

        return nullptr;
    }

    Appearance* MeshNodeImpl::getAppearance()
    {
        // non-const version of getAppearance cast to its const version to avoid duplicating code
        return const_cast<Appearance*>((const_cast<const MeshNodeImpl&>(*this)).getAppearance());
    }

    const GeometryBindingImpl* MeshNodeImpl::getGeometryBindingImpl() const
    {
        return m_geometryImpl;
    }

    const GeometryBinding* MeshNodeImpl::getGeometryBinding() const
    {
        if (m_geometryImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo<GeometryBinding>(m_geometryImpl->getRamsesObject());
        }

        return nullptr;
    }

    GeometryBinding* MeshNodeImpl::getGeometryBinding()
    {
        // non-const version of getGeometryBinding cast to its const version to avoid duplicating code
        return const_cast<GeometryBinding*>((const_cast<const MeshNodeImpl&>(*this)).getGeometryBinding());
    }

    uint32_t MeshNodeImpl::getStartIndex() const
    {
        return getIScene().getRenderable(m_renderableHandle).startIndex;
    }

    uint32_t MeshNodeImpl::getIndexCount() const
    {
        return getIScene().getRenderable(m_renderableHandle).indexCount;
    }

    EVisibilityMode MeshNodeImpl::getFlattenedVisibility() const
    {
        return VisibilityModeUtils::ConvertToHL(getIScene().getRenderable(m_renderableHandle).visibilityMode);
    }

    uint32_t MeshNodeImpl::getInstanceCount() const
    {
        return getIScene().getRenderable(m_renderableHandle).instanceCount;
    }

    bool MeshNodeImpl::AreGeometryAndAppearanceCompatible(const GeometryBindingImpl& geometry, const AppearanceImpl& appearance)
    {
        return geometry.getEffectHash() == appearance.getEffectImpl()->getLowlevelResourceHash();
    }

    uint32_t MeshNodeImpl::getStartVertex() const
    {
        return getIScene().getRenderable(m_renderableHandle).startVertex;
    }
}
