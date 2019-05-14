//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "NodeImpl.h"
#include "SceneImpl.h"
#include "ramses-client-api/Node.h"
#include "SerializationContext.h"
#include "RamsesObjectTypeUtils.h"
#include "Scene/ClientScene.h"
#include "Math3d/Vector3.h"

namespace ramses
{
    static const ramses_internal::Vector3 IdentityTranslation(0.0f, 0.0f, 0.0f);
    static const ramses_internal::Vector3 IdentityRotation(0.0f, 0.0f, 0.0f);
    static const ramses_internal::Vector3 IdentityScaling(1.0f, 1.0f, 1.0f);

    NodeImpl::NodeImpl(SceneImpl& scene, ERamsesObjectType type, const char* nodeName)
        : SceneObjectImpl(scene, type, nodeName)
        , m_parent(0)
        , m_visibility(true)
    {
    }

    NodeImpl::~NodeImpl()
    {
    }

    status_t NodeImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_nodeHandle;
        outStream << static_cast<ramses_internal::UInt32>(m_visibility);
        outStream << m_transformHandle;

        return StatusOK;
    }

    status_t NodeImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_nodeHandle;
        ramses_internal::UInt32 intToBool;
        inStream >> intToBool;
        m_visibility = (intToBool != 0);
        inStream >> m_transformHandle;

        serializationContext.addForDependencyResolve(this);
        serializationContext.addNodeHandleToNodeImplMapping(m_nodeHandle, this);

        return StatusOK;
    }

    status_t NodeImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        const ramses_internal::NodeHandle parentNodeHandle = getIScene().getParent(m_nodeHandle);
        if (parentNodeHandle.isValid())
        {
            m_parent = serializationContext.getNodeImplForHandle(parentNodeHandle);
            if (m_parent == NULL)
            {
                return addErrorEntry("Node::resolveDeserializationDependencies failed, m_parent is NULL.");
            }
        }

        const ramses_internal::UInt32 childCount = getIScene().getChildCount(m_nodeHandle);
        m_children.reserve(childCount);
        for (ramses_internal::UInt32 i = 0; i < childCount; i++)
        {
            const ramses_internal::NodeHandle childNodeHandle = getIScene().getChild(m_nodeHandle, i);
            NodeImpl* childNode = serializationContext.getNodeImplForHandle(childNodeHandle);
            if (childNode == NULL)
            {
                return addErrorEntry("Node::resolveDeserializationDependencies failed, childNode is NULL.");
            }
            m_children.push_back(childNode);

        }

        return StatusOK;
    }

    void NodeImpl::initializeFrameworkData()
    {
        m_nodeHandle = getIScene().allocateNode(0u, ramses_internal::NodeHandle::Invalid());
    }

    void NodeImpl::deinitializeFrameworkData()
    {
        if (m_transformHandle.isValid())
        {
            getIScene().releaseTransform(m_transformHandle);
            m_transformHandle = ramses_internal::TransformHandle::Invalid();
        }

        assert(m_nodeHandle.isValid());
        getIScene().releaseNode(m_nodeHandle);
        m_nodeHandle = ramses_internal::NodeHandle::Invalid();
    }

    bool NodeImpl::hasChild() const
    {
        return !m_children.empty();
    }

    uint32_t NodeImpl::getChildCount() const
    {
        return static_cast<uint32_t>(m_children.size());
    }

    Node* NodeImpl::getChild(uint32_t index)
    {
        // Use const version of getChild to avoid code duplication
        return const_cast<Node*>(const_cast<const NodeImpl&>(*this).getChild(index));
    }

    const Node* NodeImpl::getChild(uint32_t index) const
    {
        if (index >= static_cast<uint32_t>(m_children.size()))
        {
            return NULL;
        }

        const NodeImpl& child = getChildImpl(index);
        return &RamsesObjectTypeUtils::ConvertTo<Node>(child.getRamsesObject());
    }

    NodeImpl& NodeImpl::getChildImpl(uint32_t index)
    {
        return const_cast<NodeImpl&>(const_cast<const NodeImpl&>(*this).getChildImpl(index));
    }

    const NodeImpl& NodeImpl::getChildImpl(uint32_t index) const
    {
        assert(index < m_children.size());
        return *m_children[index];
    }

    status_t NodeImpl::removeChild(NodeImpl& node)
    {
        const auto it = ramses_internal::find_c(m_children, &node);
        if (it == m_children.end())
        {
            return node.addErrorEntry("Node::removeChild failed, child not found.");
        }

        removeChildInternally(it);

        return StatusOK;
    }

    status_t NodeImpl::removeAllChildren()
    {
        while (!m_children.empty())
        {
            removeChildInternally(m_children.end() - 1);
        }

        return StatusOK;
    }

    bool NodeImpl::hasParent() const
    {
        return (m_parent != NULL);
    }

    Node* NodeImpl::getParent()
    {
        return m_parent ? &RamsesObjectTypeUtils::ConvertTo<Node>(m_parent->getRamsesObject()) : NULL;
    }

    const Node* NodeImpl::getParent() const
    {
        return m_parent ? &RamsesObjectTypeUtils::ConvertTo<Node>(m_parent->getRamsesObject()) : NULL;
    }

    NodeImpl* NodeImpl::getParentImpl()
    {
        return m_parent;
    }

    const NodeImpl* NodeImpl::getParentImpl() const
    {
        return m_parent;
    }

    status_t NodeImpl::addChild(NodeImpl& childNode)
    {
        if (!isFromTheSameSceneAs(childNode))
        {
            return addErrorEntry("Node::addChildToNode failed, nodes were created in different scenes.");
        }

        if (this == &childNode)
        {
            return addErrorEntry("Node::addChildToNode failed, trying to reparent node to itself.");
        }

        if (childNode.hasParent() && (childNode.removeParent() != StatusOK))
        {
            return addErrorEntry("Node::addChildToNode failed, failed to remove current parent.");
        }

        childNode.markDirty();

        // on low level
        getIScene().addChildToNode(m_nodeHandle, childNode.m_nodeHandle);

        // on high level
        childNode.m_parent = this;
        m_children.push_back(&childNode);

        return StatusOK;
    }

    status_t NodeImpl::setParent(NodeImpl& parentNode)
    {
        return parentNode.addChild(*this);
    }

    status_t NodeImpl::removeParent()
    {
        if (!hasParent())
        {
            return addErrorEntry("Node::removeParent failed, node has no parent.");
        }

        return m_parent->removeChild(*this);
    }

    status_t NodeImpl::getModelMatrix(float(&modelMatrix)[16]) const
    {
        const ramses_internal::Matrix44f mat44 = getIScene().updateMatrixCache(ramses_internal::ETransformationMatrixType_World, m_nodeHandle);
        ramses_internal::PlatformMemory::Copy(modelMatrix, mat44.data, sizeof(modelMatrix));
        return StatusOK;
    }

    status_t NodeImpl::getInverseModelMatrix(float(&inverseModelMatrix)[16]) const
    {
        const ramses_internal::Matrix44f mat44 = getIScene().updateMatrixCache(ramses_internal::ETransformationMatrixType_Object, m_nodeHandle);
        ramses_internal::PlatformMemory::Copy(inverseModelMatrix, mat44.data, sizeof(inverseModelMatrix));
        return StatusOK;
    }

    ramses_internal::SceneId NodeImpl::getSceneId() const
    {
        return getIScene().getSceneId();
    }

    ramses_internal::NodeHandle NodeImpl::getNodeHandle() const
    {
        return m_nodeHandle;
    }

    void NodeImpl::markDirty()
    {
        getSceneImpl().getObjectRegistry().setNodeDirty(*this, true);
    }

    bool NodeImpl::isDirty() const
    {
        return getSceneImpl().getObjectRegistry().isNodeDirty(*this);
    }

    void NodeImpl::removeChildInternally(NodeVector::iterator childIt)
    {
        NodeImpl& child = **childIt;

        m_children.erase(childIt);
        child.m_parent = NULL;
        child.markDirty();

        getIScene().removeChildFromNode(m_nodeHandle, child.m_nodeHandle);
    }

    status_t NodeImpl::translate(float x, float y, float z)
    {
        if (!m_transformHandle.isValid())
        {
            if (ramses_internal::Vector3(x, y, z) == IdentityTranslation)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const ramses_internal::Vector3& previous = getIScene().getTranslation(m_transformHandle);
        getIScene().setTranslation(m_transformHandle, previous + ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t NodeImpl::setTranslation(float x, float y, float z)
    {
        if (!m_transformHandle.isValid())
        {
            if (ramses_internal::Vector3(x, y, z) == IdentityTranslation)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const ramses_internal::Vector3 newValue(x, y, z);
        if (newValue != getIScene().getTranslation(m_transformHandle))
        {
            getIScene().setTranslation(m_transformHandle, newValue);
        }
        return StatusOK;
    }

    status_t NodeImpl::getTranslation(float& x, float& y, float& z) const
    {
        if (!m_transformHandle.isValid())
        {
            x = IdentityTranslation.x;
            y = IdentityTranslation.y;
            z = IdentityTranslation.z;
            return StatusOK;
        }

        const ramses_internal::Vector3& value = getIScene().getTranslation(m_transformHandle);
        x = value[0];
        y = value[1];
        z = value[2];
        return StatusOK;
    }

    status_t NodeImpl::rotate(float x, float y, float z)
    {
        if (!m_transformHandle.isValid())
        {
            if (ramses_internal::Vector3(x, y, z) == IdentityRotation)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const ramses_internal::Vector3& previous = getIScene().getRotation(m_transformHandle);
        getIScene().setRotation(m_transformHandle, previous + ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t NodeImpl::setRotation(float x, float y, float z)
    {
        if (!m_transformHandle.isValid())
        {
            if (ramses_internal::Vector3(x, y, z) == IdentityRotation)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const ramses_internal::Vector3 newValue(x, y, z);
        if (newValue != getIScene().getRotation(m_transformHandle))
        {
            getIScene().setRotation(m_transformHandle, newValue);
        }
        return StatusOK;
    }

    status_t NodeImpl::getRotation(float& x, float& y, float& z) const
    {
        if (!m_transformHandle.isValid())
        {
            x = IdentityRotation.x;
            y = IdentityRotation.y;
            z = IdentityRotation.z;
            return StatusOK;
        }

        const ramses_internal::Vector3& value = getIScene().getRotation(m_transformHandle);
        x = value[0];
        y = value[1];
        z = value[2];
        return StatusOK;
    }

    status_t NodeImpl::scale(float x, float y, float z)
    {
        if (!m_transformHandle.isValid())
        {
            if (ramses_internal::Vector3(x, y, z) == IdentityScaling)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const ramses_internal::Vector3& previous = getIScene().getScaling(m_transformHandle);
        getIScene().setScaling(m_transformHandle, previous * ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t NodeImpl::setScaling(float x, float y, float z)
    {
        if (!m_transformHandle.isValid())
        {
            if (ramses_internal::Vector3(x, y, z) == IdentityScaling)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const ramses_internal::Vector3 newValue(x, y, z);
        if (newValue != getIScene().getScaling(m_transformHandle))
        {
            getIScene().setScaling(m_transformHandle, newValue);
        }
        return StatusOK;
    }

    status_t NodeImpl::getScaling(float& x, float& y, float& z) const
    {
        if (!m_transformHandle.isValid())
        {
            x = IdentityScaling.x;
            y = IdentityScaling.y;
            z = IdentityScaling.z;
            return StatusOK;
        }

        const ramses_internal::Vector3& value = getIScene().getScaling(m_transformHandle);
        x = value[0];
        y = value[1];
        z = value[2];
        return StatusOK;
    }

    void NodeImpl::initializeTransform()
    {
        if (!m_transformHandle.isValid())
        {
            m_transformHandle = getIScene().allocateTransform(getNodeHandle(), ramses_internal::TransformHandle::Invalid());
        }
    }

    ramses_internal::TransformHandle NodeImpl::getTransformHandle() const
    {
        return m_transformHandle;
    }

    status_t NodeImpl::setVisibility(bool visible)
    {
        if (m_visibility != visible)
        {
            m_visibility = visible;
            markDirty();
        }

        return StatusOK;
    }

    bool NodeImpl::getVisibility() const
    {
        return m_visibility;
    }
}
