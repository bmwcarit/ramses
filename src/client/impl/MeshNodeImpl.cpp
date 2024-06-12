//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "impl/MeshNodeImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"

// API
#include "ramses/client/Geometry.h"

// impls
#include "impl/AppearanceImpl.h"
#include "impl/EffectImpl.h"
#include "impl/ArrayResourceImpl.h"
#include "impl/AppearanceImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SerializationContext.h"
#include "impl/ErrorReporting.h"

// internal
#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    MeshNodeImpl::MeshNodeImpl(SceneImpl& scene, std::string_view nodeName)
        : NodeImpl(scene, ERamsesObjectType::MeshNode, nodeName)
        , m_appearanceImpl(nullptr)
        , m_geometryImpl(nullptr)
    {
    }

    MeshNodeImpl::~MeshNodeImpl() = default;

    bool MeshNodeImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!NodeImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_renderableHandle;
        outStream << (m_appearanceImpl ? serializationContext.getIDForObject(m_appearanceImpl) : SerializationContext::GetObjectIDNull());
        outStream << (m_geometryImpl ? serializationContext.getIDForObject(m_geometryImpl) : SerializationContext::GetObjectIDNull());

        return true;
    }

    bool MeshNodeImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!NodeImpl::deserialize(inStream, serializationContext))
            return false;

        serializationContext.deserializeAndMap(inStream, m_renderableHandle);

        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_appearanceImpl);
        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_geometryImpl);
        serializationContext.addForDependencyResolve(this);

        return true;
    }

    bool MeshNodeImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!NodeImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_appearanceImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_geometryImpl);

        return true;
    }

    void MeshNodeImpl::onValidate(ValidationReportImpl& report) const
    {
        NodeImpl::onValidate(report);
        if (nullptr == m_appearanceImpl)
        {
            report.add(EIssueType::Error, "meshnode does not have an appearance set", &getRamsesObject());
        }
        else
        {
            report.addDependentObject(*this, *m_appearanceImpl);
        }

        if (nullptr == m_geometryImpl)
        {
            report.add(EIssueType::Error, "meshnode does not have a geometryBinding set", &getRamsesObject());
        }
        else
        {
            report.addDependentObject(*this, *m_geometryImpl);

            const bool hasIndexArray = m_geometryImpl->getIndicesCount() > 0;
            if (hasIndexArray && (m_geometryImpl->getIndicesCount() < getStartIndex() + getIndexCount()))
                report.add(EIssueType::Error, "startIndex + indexCount exceeds indices of indexarray", &getRamsesObject());

            if (getIndexCount() == 0)
                report.add(EIssueType::Error, "indexCount must be greater 0", &getRamsesObject());
        }
    }

    void MeshNodeImpl::initializeFrameworkData()
    {
        NodeImpl::initializeFrameworkData();

        m_renderableHandle = getIScene().allocateRenderable(getNodeHandle(), ramses::internal::RenderableHandle::Invalid());
    }

    void MeshNodeImpl::deinitializeFrameworkData()
    {
        assert(m_renderableHandle.isValid());

        getIScene().releaseRenderable(m_renderableHandle);

        NodeImpl::deinitializeFrameworkData();
    }

    bool MeshNodeImpl::setAppearance(AppearanceImpl& appearance)
    {
        if (!isFromTheSameSceneAs(appearance))
        {
            getErrorReporting().set("MeshNode::setAppearance failed - appearance is not from the same scene as this MeshNode.", *this);
            return false;
        }

        if (m_appearanceImpl == &appearance)
        {
            return true;
        }

        if (m_geometryImpl != nullptr && !AreGeometryAndAppearanceCompatible(*m_geometryImpl, appearance))
        {
            getErrorReporting().set("MeshNode::setAppearance failed - previously assigned geometry does not provide all vertex attributes required by the appearance!", *this);
            return false;
        }

        m_appearanceImpl = &appearance;

        getIScene().setRenderableUniformsDataInstanceAndState(m_renderableHandle, appearance.getUniformDataInstance(), appearance.getRenderStateHandle());

        return true;
    }

    bool MeshNodeImpl::setGeometry(GeometryImpl& geometry)
    {
        if (!isFromTheSameSceneAs(geometry))
        {
            getErrorReporting().set("MeshNode::setGeometry failed - geometry is not from the same scene as this MeshNode.", *this);
            return false;
        }

        if (m_geometryImpl == &geometry)
        {
            return true;
        }

        if (m_appearanceImpl != nullptr && !AreGeometryAndAppearanceCompatible(geometry, *m_appearanceImpl))
        {
            getErrorReporting().set("MeshNode::setGeometry failed - the geometry does not provide all vertex attributes required by previously assigned appearance!", *this);
            return false;
        }

        m_geometryImpl = &geometry;

        const uint32_t numberOfIndicesFromGeometry = geometry.getIndicesCount();
        const bool geometryBindingProvidesIndexArray = numberOfIndicesFromGeometry > 0;
        if (geometryBindingProvidesIndexArray)
        {
            setStartIndex(0u);
            setIndexCount(numberOfIndicesFromGeometry);
        }
        getIScene().setRenderableDataInstance(m_renderableHandle, ramses::internal::ERenderableDataSlotType_Geometry, geometry.getAttributeDataInstance());

        return true;
    }

    bool MeshNodeImpl::removeAppearanceAndGeometry()
    {
        m_appearanceImpl = nullptr;
        getIScene().setRenderableUniformsDataInstanceAndState(m_renderableHandle, ramses::internal::DataInstanceHandle::Invalid(), ramses::internal::RenderStateHandle::Invalid());

        m_geometryImpl = nullptr;
        getIScene().setRenderableDataInstance(m_renderableHandle, ramses::internal::ERenderableDataSlotType_Geometry, ramses::internal::DataInstanceHandle::Invalid());

        return setIndexCount(0u);
    }

    bool MeshNodeImpl::setStartIndex(uint32_t startIndex)
    {
        getIScene().setRenderableStartIndex(m_renderableHandle, startIndex);
        return true;
    }

    bool MeshNodeImpl::setIndexCount(uint32_t indexCount)
    {
        getIScene().setRenderableIndexCount(m_renderableHandle, indexCount);
        return true;
    }

    bool MeshNodeImpl::setFlattenedVisibility(EVisibilityMode mode)
    {
        getIScene().setRenderableVisibility(m_renderableHandle, mode);
        return true;
    }

    bool MeshNodeImpl::setInstanceCount(uint32_t instanceCount)
    {
        getIScene().setRenderableInstanceCount(m_renderableHandle, instanceCount);
        return true;
    }

    bool MeshNodeImpl::setStartVertex(uint32_t startVertex)
    {
        getIScene().setRenderableStartVertex(m_renderableHandle, startVertex);
        return true;
    }

    ramses::internal::RenderableHandle MeshNodeImpl::getRenderableHandle() const
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

    const GeometryImpl* MeshNodeImpl::getGeometryImpl() const
    {
        return m_geometryImpl;
    }

    const Geometry* MeshNodeImpl::getGeometry() const
    {
        if (m_geometryImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo<Geometry>(m_geometryImpl->getRamsesObject());
        }

        return nullptr;
    }

    Geometry* MeshNodeImpl::getGeometry()
    {
        // non-const version of getGeometry cast to its const version to avoid duplicating code
        return const_cast<Geometry*>((const_cast<const MeshNodeImpl&>(*this)).getGeometry());
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
        return getIScene().getRenderable(m_renderableHandle).visibilityMode;
    }

    uint32_t MeshNodeImpl::getInstanceCount() const
    {
        return getIScene().getRenderable(m_renderableHandle).instanceCount;
    }

    bool MeshNodeImpl::AreGeometryAndAppearanceCompatible(const GeometryImpl& geometry, const AppearanceImpl& appearance)
    {
        return geometry.getEffectHash() == appearance.getEffectImpl()->getLowlevelResourceHash();
    }

    uint32_t MeshNodeImpl::getStartVertex() const
    {
        return getIScene().getRenderable(m_renderableHandle).startVertex;
    }
}
