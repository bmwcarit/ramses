//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/Node.h"
#include "impl/NodeImpl.h"
#include "impl/SceneImpl.h"
#include "impl/SerializationContext.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SceneObjectRegistry.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    NodeImpl::NodeImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view nodeName)
        : SceneObjectImpl(scene, type, nodeName)
        , m_parent(nullptr)
        , m_visibilityMode(EVisibilityMode::Visible)
    {
    }

    NodeImpl::~NodeImpl() = default;

    bool NodeImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_nodeHandle;
        outStream << m_visibilityMode;
        outStream << m_transformHandle;

        return true;
    }

    bool NodeImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        serializationContext.deserializeAndMap(inStream, m_nodeHandle);
        inStream >> m_visibilityMode;
        serializationContext.deserializeAndMap(inStream, m_transformHandle);

        serializationContext.addForDependencyResolve(this);
        serializationContext.addNodeHandleToNodeImplMapping(m_nodeHandle, this);

        return true;
    }

    bool NodeImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        const ramses::internal::NodeHandle parentNodeHandle = getIScene().getParent(m_nodeHandle);
        if (parentNodeHandle.isValid())
        {
            m_parent = serializationContext.getNodeImplForHandle(parentNodeHandle);
            if (m_parent == nullptr)
            {
                getErrorReporting().set("Node::resolveDeserializationDependencies failed, m_parent is NULL.");
                return false;
            }
        }

        const uint32_t childCount = getIScene().getChildCount(m_nodeHandle);
        m_children.reserve(childCount);
        for (uint32_t i = 0; i < childCount; i++)
        {
            const ramses::internal::NodeHandle childNodeHandle = getIScene().getChild(m_nodeHandle, i);
            NodeImpl* childNode = serializationContext.getNodeImplForHandle(childNodeHandle);
            if (childNode == nullptr)
            {
                getErrorReporting().set("Node::resolveDeserializationDependencies failed, childNode is NULL.");
                return false;
            }
            m_children.push_back(childNode);

        }

        return true;
    }

    void NodeImpl::initializeFrameworkData()
    {
        m_nodeHandle = getIScene().allocateNode(0u, ramses::internal::NodeHandle::Invalid());
    }

    void NodeImpl::deinitializeFrameworkData()
    {
        if (m_transformHandle.isValid())
        {
            getIScene().releaseTransform(m_transformHandle);
            m_transformHandle = ramses::internal::TransformHandle::Invalid();
        }

        assert(m_nodeHandle.isValid());
        getIScene().releaseNode(m_nodeHandle);
        m_nodeHandle = ramses::internal::NodeHandle::Invalid();
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

    bool NodeImpl::removeChild(NodeImpl& node)
    {
        const auto it = ramses::internal::find_c(m_children, &node);
        if (it == m_children.end())
        {
            getErrorReporting().set("Node::removeChild failed, child not found.", *this);
            return false;
        }

        removeChildInternally(it);

        return true;
    }

    bool NodeImpl::removeAllChildren()
    {
        while (!m_children.empty())
        {
            removeChildInternally(m_children.end() - 1);
        }

        return true;
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

    bool NodeImpl::addChild(NodeImpl& childNode)
    {
        if (!isFromTheSameSceneAs(childNode))
        {
            getErrorReporting().set("Node::addChildToNode failed, nodes were created in different scenes.", *this);
            return false;
        }

        if (this == &childNode)
        {
            getErrorReporting().set("Node::addChildToNode failed, trying to reparent node to itself.", *this);
            return false;
        }

        if (childNode.hasParent() && !childNode.removeParent())
        {
            getErrorReporting().set("Node::addChildToNode failed, failed to remove current parent.", *this);
            return false;
        }

        childNode.markDirty();

        // on low level
        getIScene().addChildToNode(m_nodeHandle, childNode.m_nodeHandle);

        // on high level
        childNode.m_parent = this;
        m_children.push_back(&childNode);

        return true;
    }

    bool NodeImpl::setParent(NodeImpl& parentNode)
    {
        return parentNode.addChild(*this);
    }

    bool NodeImpl::removeParent()
    {
        if (!hasParent())
        {
            getErrorReporting().set("Node::removeParent failed, node has no parent.", *this);
            return false;
        }

        return m_parent->removeChild(*this);
    }

    bool NodeImpl::getModelMatrix(matrix44f& modelMatrix) const
    {
        modelMatrix = getIScene().updateMatrixCache(ramses::internal::ETransformationMatrixType_World, m_nodeHandle);
        return true;
    }

    bool NodeImpl::getInverseModelMatrix(matrix44f& inverseModelMatrix) const
    {
        inverseModelMatrix = getIScene().updateMatrixCache(ramses::internal::ETransformationMatrixType_Object, m_nodeHandle);
        return true;
    }

    ramses::internal::SceneId NodeImpl::getSceneId() const
    {
        return getIScene().getSceneId();
    }

    ramses::internal::NodeHandle NodeImpl::getNodeHandle() const
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

    bool NodeImpl::translate(const vec3f& translation)
    {
        if (!m_transformHandle.isValid())
        {
            if (translation == ramses::internal::IScene::IdentityTranslation)
            {
                return true;
            }
            initializeTransform();
        }
        const auto& previous = getIScene().getTranslation(m_transformHandle);
        getIScene().setTranslation(m_transformHandle, previous + glm::vec3(translation));
        return true;
    }

    bool NodeImpl::setTranslation(const vec3f& translation)
    {
        if (!m_transformHandle.isValid())
        {
            if (translation == ramses::internal::IScene::IdentityTranslation)
            {
                return true;
            }
            initializeTransform();
        }
        if (translation != getIScene().getTranslation(m_transformHandle))
        {
            getIScene().setTranslation(m_transformHandle, translation);
        }
        return true;
    }

    bool NodeImpl::getTranslation(vec3f& translation) const
    {
        if (!m_transformHandle.isValid())
        {
            translation = ramses::internal::IScene::IdentityTranslation;
            return true;
        }

        translation = getIScene().getTranslation(m_transformHandle);
        return true;
    }

    bool NodeImpl::setRotation(const vec3f& rotation, ERotationType rotationType)
    {
        if (rotationType == ERotationType::Quaternion)
        {
            getErrorReporting().set("Invalid rotation rotationType: Quaternion", *this);
            return false;
        }
        return setRotationInternal({rotation.x, rotation.y, rotation.z, 1.f}, rotationType);
    }

    bool NodeImpl::setRotationInternal(glm::vec4&& rotation, ERotationType rotationType)
    {
        if (!m_transformHandle.isValid())
        {
            if (rotation == ramses::internal::IScene::IdentityRotation)
            {
                return true;
            }
            initializeTransform();
        }

        if (rotation != getIScene().getRotation(m_transformHandle)
            || rotationType != getIScene().getRotationType(m_transformHandle))
        {
            getIScene().setRotation(m_transformHandle, rotation, rotationType);
        }

        return true;
    }

    ERotationType NodeImpl::getRotationType() const
    {
        if (!m_transformHandle.isValid())
        {
            return ERotationType::Euler_XYZ;
        }
        return getIScene().getRotationType(m_transformHandle);
    }

    bool NodeImpl::getRotation(vec3f& rotation) const
    {
        if (!m_transformHandle.isValid())
        {
            rotation = ramses::internal::IScene::IdentityRotation;
            return true;
        }

        if (ERotationType::Quaternion == getIScene().getRotationType(m_transformHandle))
        {
            getErrorReporting().set("Node::getRotation(vec3f&) failed: rotation was set by quaternion before. Check Node::getRotationType().", *this);
            return false;
        }

        rotation = getIScene().getRotation(m_transformHandle);
        return true;
    }

    bool NodeImpl::setRotation(const quat& rotation)
    {
        glm::vec4 vec{rotation.x, rotation.y, rotation.z, rotation.w};
        return setRotationInternal(std::move(vec), ERotationType::Quaternion);
    }

    bool NodeImpl::getRotation(quat& rotation) const
    {
        if (!m_transformHandle.isValid())
        {
            rotation = glm::identity<quat>();
            return true;
        }

        if (ERotationType::Quaternion != getIScene().getRotationType(m_transformHandle))
        {
            getErrorReporting().set("Node::getRotation(quat&) failed: rotation was set by euler angles before. Check Node::getRotationType().", *this);
            return false;
        }

        const auto& value = getIScene().getRotation(m_transformHandle);
        rotation = quat(value.w, value.x, value.y, value.z);
        return true;
    }

    bool NodeImpl::scale(const vec3f& scaling)
    {
        if (!m_transformHandle.isValid())
        {
            if (scaling == ramses::internal::IScene::IdentityScaling)
            {
                return true;
            }
            initializeTransform();
        }
        const auto& previous = getIScene().getScaling(m_transformHandle);
        getIScene().setScaling(m_transformHandle, previous * scaling);

        return true;
    }

    bool NodeImpl::setScaling(const vec3f& scaling)
    {
        if (!m_transformHandle.isValid())
        {
            if (scaling == ramses::internal::IScene::IdentityScaling)
            {
                return true;
            }
            initializeTransform();
        }
        if (scaling != getIScene().getScaling(m_transformHandle))
        {
            getIScene().setScaling(m_transformHandle, scaling);
        }

        return true;
    }

    bool NodeImpl::getScaling(vec3f& scaling) const
    {
        if (!m_transformHandle.isValid())
        {
            scaling = ramses::internal::IScene::IdentityScaling;
            return true;
        }

        scaling = getIScene().getScaling(m_transformHandle);
        return true;
    }

    void NodeImpl::initializeTransform()
    {
        if (!m_transformHandle.isValid())
        {
            m_transformHandle = getIScene().allocateTransform(getNodeHandle(), ramses::internal::TransformHandle::Invalid());
        }
    }

    ramses::internal::TransformHandle NodeImpl::getTransformHandle() const
    {
        return m_transformHandle;
    }

    bool NodeImpl::setVisibility(EVisibilityMode mode)
    {
        if (m_visibilityMode != mode)
        {
            m_visibilityMode = mode;
            markDirty();
        }

        return true;
    }

    EVisibilityMode NodeImpl::getVisibility() const
    {
        return m_visibilityMode;
    }
}
