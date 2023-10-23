//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Node.h"

// internal
#include "impl/NodeImpl.h"
#include "internal/VisibilityModeUtils.h"

namespace ramses
{
    Node::Node(std::unique_ptr<internal::NodeImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::NodeImpl&>(SceneObject::m_impl) }
    {
    }

    bool Node::hasChild() const
    {
        return m_impl.hasChild();
    }

    size_t Node::getChildCount() const
    {
        return m_impl.getChildCount();
    }

    Node* Node::getChild(size_t index)
    {
        return m_impl.getChild(index);
    }

    const Node* Node::getChild(size_t index) const
    {
        return m_impl.getChild(index);
    }

    bool Node::addChild(Node& node)
    {
        const bool status = m_impl.addChild(node.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(node));
        return status;
    }

    bool Node::removeChild(Node& node)
    {
        const bool status = m_impl.removeChild(node.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(node));
        return status;
    }

    bool Node::removeAllChildren()
    {
        const bool status =  m_impl.removeAllChildren();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool Node::hasParent() const
    {
        return m_impl.hasParent();
    }

    Node* Node::getParent()
    {
        return m_impl.getParent();
    }

    const Node* Node::getParent() const
    {
        return m_impl.getParent();
    }

    bool Node::setParent(Node& node)
    {
        const bool status = m_impl.setParent(node.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(node));
        return status;
    }

    bool Node::removeParent()
    {
        return m_impl.removeParent();
    }

    bool Node::getModelMatrix(matrix44f& modelMatrix) const
    {
        return m_impl.getModelMatrix(modelMatrix);
    }

    bool Node::getInverseModelMatrix(matrix44f& inverseModelMatrix) const
    {
        return m_impl.getInverseModelMatrix(inverseModelMatrix);
    }

    bool Node::setRotation(const vec3f& rotation, ERotationType rotationType)
    {
        const bool status = m_impl.setRotation(rotation, rotationType);
        LOG_HL_CLIENT_API4(status, rotation.x, rotation.y, rotation.z, rotationType);
        return status;
    }

    bool Node::setRotation(const quat& rotation)
    {
        const bool status = m_impl.setRotation(rotation);
        LOG_HL_CLIENT_API4(status, rotation.w, rotation.x, rotation.y, rotation.z);
        return status;
    }

    ERotationType Node::getRotationType() const
    {
        return m_impl.getRotationType();
    }

    bool Node::getRotation(vec3f& rotation) const
    {
        return m_impl.getRotation(rotation);
    }

    bool Node::getRotation(quat& rotation) const
    {
        return m_impl.getRotation(rotation);
    }

    bool Node::translate(const vec3f& translation)
    {
        const bool status = m_impl.translate(translation);
        LOG_HL_CLIENT_API3(status, translation.x, translation.y, translation.z);
        return status;
    }

    bool Node::setTranslation(const vec3f& translation)
    {
        const bool status = m_impl.setTranslation(translation);
        LOG_HL_CLIENT_API3(status, translation.x, translation.y, translation.z);
        return status;
    }

    bool Node::getTranslation(vec3f& translation) const
    {
        return m_impl.getTranslation(translation);
    }

    bool Node::scale(const vec3f& scaling)
    {
        const bool status = m_impl.scale(scaling);
        LOG_HL_CLIENT_API3(status, scaling.x, scaling.y, scaling.z);
        return status;
    }

    bool Node::setScaling(const vec3f& scaling)
    {
        const bool status = m_impl.setScaling(scaling);
        LOG_HL_CLIENT_API3(status, scaling.x, scaling.y, scaling.z);
        return status;
    }

    bool Node::getScaling(vec3f& scaling) const
    {
        return m_impl.getScaling(scaling);
    }

    bool Node::setVisibility(EVisibilityMode mode)
    {
        const bool status = m_impl.setVisibility(mode);
        LOG_HL_CLIENT_API1(status, ramses::internal::EnumToString(mode));
        return status;
    }

    EVisibilityMode Node::getVisibility() const
    {
        return m_impl.getVisibility();
    }

    internal::NodeImpl& Node::impl()
    {
        return m_impl;
    }

    const internal::NodeImpl& Node::impl() const
    {
        return m_impl;
    }
}
