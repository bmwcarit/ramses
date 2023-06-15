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
#include "RotationTypeUtils.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses
{
    NodeImpl::NodeImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view nodeName)
        : SceneObjectImpl(scene, type, nodeName)
        , m_parent(nullptr)
        , m_visibilityMode(EVisibilityMode::Visible)
    {
    }

    NodeImpl::~NodeImpl()
    {
    }

    status_t NodeImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_nodeHandle;
        outStream << m_visibilityMode;
        outStream << m_transformHandle;

        return StatusOK;
    }

    status_t NodeImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_nodeHandle;
        inStream >> m_visibilityMode;
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
            if (m_parent == nullptr)
            {
                return addErrorEntry("Node::resolveDeserializationDependencies failed, m_parent is NULL.");
            }
        }

        const uint32_t childCount = getIScene().getChildCount(m_nodeHandle);
        m_children.reserve(childCount);
        for (uint32_t i = 0; i < childCount; i++)
        {
            const ramses_internal::NodeHandle childNodeHandle = getIScene().getChild(m_nodeHandle, i);
            NodeImpl* childNode = serializationContext.getNodeImplForHandle(childNodeHandle);
            if (childNode == nullptr)
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

    size_t NodeImpl::getChildCount() const
    {
        return m_children.size();
    }

    Node* NodeImpl::getChild(size_t index)
    {
        // Use const version of getChild to avoid code duplication
        return const_cast<Node*>(const_cast<const NodeImpl&>(*this).getChild(index));
    }

    const Node* NodeImpl::getChild(size_t index) const
    {
        if (index >= m_children.size())
        {
            return nullptr;
        }

        const NodeImpl& child = getChildImpl(index);
        return &RamsesObjectTypeUtils::ConvertTo<Node>(child.getRamsesObject());
    }

    NodeImpl& NodeImpl::getChildImpl(size_t index)
    {
        return const_cast<NodeImpl&>(const_cast<const NodeImpl&>(*this).getChildImpl(index));
    }

    const NodeImpl& NodeImpl::getChildImpl(size_t index) const
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
        return (m_parent != nullptr);
    }

    Node* NodeImpl::getParent()
    {
        return m_parent ? &RamsesObjectTypeUtils::ConvertTo<Node>(m_parent->getRamsesObject()) : nullptr;
    }

    const Node* NodeImpl::getParent() const
    {
        return m_parent ? &RamsesObjectTypeUtils::ConvertTo<Node>(m_parent->getRamsesObject()) : nullptr;
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

    status_t NodeImpl::getModelMatrix(matrix44f& modelMatrix) const
    {
        modelMatrix = getIScene().updateMatrixCache(ramses_internal::ETransformationMatrixType_World, m_nodeHandle);
        return StatusOK;
    }

    status_t NodeImpl::getInverseModelMatrix(matrix44f& inverseModelMatrix) const
    {
        inverseModelMatrix = getIScene().updateMatrixCache(ramses_internal::ETransformationMatrixType_Object, m_nodeHandle);
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
        child.m_parent = nullptr;
        child.markDirty();

        getIScene().removeChildFromNode(m_nodeHandle, child.m_nodeHandle);
    }

    status_t NodeImpl::translate(const vec3f& translation)
    {
        if (!m_transformHandle.isValid())
        {
            if (translation == ramses_internal::IScene::IdentityTranslation)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const auto& previous = getIScene().getTranslation(m_transformHandle);
        getIScene().setTranslation(m_transformHandle, previous + glm::vec3(translation));
        return StatusOK;
    }

    status_t NodeImpl::setTranslation(const vec3f& translation)
    {
        if (!m_transformHandle.isValid())
        {
            if (translation == ramses_internal::IScene::IdentityTranslation)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        if (translation != getIScene().getTranslation(m_transformHandle))
        {
            getIScene().setTranslation(m_transformHandle, translation);
        }
        return StatusOK;
    }

    status_t NodeImpl::getTranslation(vec3f& translation) const
    {
        if (!m_transformHandle.isValid())
        {
            translation = ramses_internal::IScene::IdentityTranslation;
            return StatusOK;
        }

        translation = getIScene().getTranslation(m_transformHandle);
        return StatusOK;
    }

    ramses::status_t NodeImpl::setRotation(const vec3f& rotation, ERotationType rotationType)
    {
        if (rotationType == ERotationType::Quaternion)
        {
            return addErrorEntry("Invalid rotation rotationType: Quaternion");
        }
        const auto rotationConventionInternal = RotationTypeUtils::ConvertRotationTypeToInternal(rotationType);
        return setRotationInternal({rotation.x, rotation.y, rotation.z, 1.f}, rotationConventionInternal);
    }

    ramses::status_t NodeImpl::setRotationInternal(glm::vec4&& rotation, ramses_internal::ERotationType rotationType)
    {
        if (!m_transformHandle.isValid())
        {
            if (rotation == ramses_internal::IScene::IdentityRotation)
            {
                return StatusOK;
            }
            initializeTransform();
        }

        if (rotation != getIScene().getRotation(m_transformHandle)
            || rotationType != getIScene().getRotationType(m_transformHandle))
        {
            getIScene().setRotation(m_transformHandle, rotation, rotationType);
        }

        return StatusOK;
    }

    ERotationType NodeImpl::getRotationType() const
    {
        if (!m_transformHandle.isValid())
        {
            return ERotationType::Euler_XYZ;
        }
        const auto rotationConventionInternal = getIScene().getRotationType(m_transformHandle);
        return RotationTypeUtils::ConvertRotationTypeFromInternal(rotationConventionInternal);
    }

    status_t NodeImpl::getRotation(vec3f& rotation) const
    {
        if (!m_transformHandle.isValid())
        {
            rotation = ramses_internal::IScene::IdentityRotation;
            return StatusOK;
        }

        if (ramses_internal::ERotationType::Quaternion == getIScene().getRotationType(m_transformHandle))
        {
            return addErrorEntry("Node::getRotation(vec3f&) failed: rotation was set by quaternion before. Check Node::getRotationType().");
        }

        rotation = getIScene().getRotation(m_transformHandle);
        return StatusOK;
    }

    status_t NodeImpl::setRotation(const quat& rotation)
    {
        glm::vec4 vec{rotation.x, rotation.y, rotation.z, rotation.w};
        return setRotationInternal(std::move(vec), ramses_internal::ERotationType::Quaternion);
    }

    ramses::status_t NodeImpl::getRotation(quat& rotation) const
    {
        if (!m_transformHandle.isValid())
        {
            rotation = glm::identity<quat>();
            return StatusOK;
        }

        if (ramses_internal::ERotationType::Quaternion != getIScene().getRotationType(m_transformHandle))
        {
            return addErrorEntry("Node::getRotation(quat&) failed: rotation was set by euler angles before. Check Node::getRotationType().");
        }

        const auto& value = getIScene().getRotation(m_transformHandle);
        rotation = quat(value.w, value.x, value.y, value.z);
        return StatusOK;
    }

    status_t NodeImpl::scale(const vec3f& scaling)
    {
        if (!m_transformHandle.isValid())
        {
            if (scaling == ramses_internal::IScene::IdentityScaling)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        const auto& previous = getIScene().getScaling(m_transformHandle);
        getIScene().setScaling(m_transformHandle, previous * scaling);

        return StatusOK;
    }

    status_t NodeImpl::setScaling(const vec3f& scaling)
    {
        if (!m_transformHandle.isValid())
        {
            if (scaling == ramses_internal::IScene::IdentityScaling)
            {
                return StatusOK;
            }
            initializeTransform();
        }
        if (scaling != getIScene().getScaling(m_transformHandle))
        {
            getIScene().setScaling(m_transformHandle, scaling);
        }

        return StatusOK;
    }

    status_t NodeImpl::getScaling(vec3f& scaling) const
    {
        if (!m_transformHandle.isValid())
        {
            scaling = ramses_internal::IScene::IdentityScaling;
            return StatusOK;
        }

        scaling = getIScene().getScaling(m_transformHandle);
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

    status_t NodeImpl::setVisibility(EVisibilityMode mode)
    {
        if (m_visibilityMode != mode)
        {
            m_visibilityMode = mode;
            markDirty();
        }

        return StatusOK;
    }

    EVisibilityMode NodeImpl::getVisibility() const
    {
        return m_visibilityMode;
    }
}
