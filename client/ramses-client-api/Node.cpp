//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Node.h"

// internal
#include "NodeImpl.h"
#include "VisibilityModeUtils.h"

namespace ramses
{
    Node::Node(NodeImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    Node::~Node()
    {
    }

    bool Node::hasChild() const
    {
        return impl.hasChild();
    }

    uint32_t Node::getChildCount() const
    {
        return impl.getChildCount();
    }

    Node* Node::getChild(uint32_t index)
    {
        return impl.getChild(index);
    }

    const Node* Node::getChild(uint32_t index) const
    {
        return impl.getChild(index);
    }

    status_t Node::addChild(Node& node)
    {
        const status_t status = impl.addChild(node.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(node));
        return status;
    }

    status_t Node::removeChild(Node& node)
    {
        const status_t status = impl.removeChild(node.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(node));
        return status;
    }

    status_t Node::removeAllChildren()
    {
        const status_t status =  impl.removeAllChildren();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool Node::hasParent() const
    {
        return impl.hasParent();
    }

    Node* Node::getParent()
    {
        return impl.getParent();
    }

    const Node* Node::getParent() const
    {
        return impl.getParent();
    }

    status_t Node::setParent(Node& node)
    {
        const status_t status = impl.setParent(node.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(node));
        return status;
    }

    status_t Node::removeParent()
    {
        return impl.removeParent();
    }

    status_t Node::getModelMatrix(matrix44f& modelMatrix) const
    {
        return impl.getModelMatrix(modelMatrix);
    }

    status_t Node::getInverseModelMatrix(matrix44f& inverseModelMatrix) const
    {
        return impl.getInverseModelMatrix(inverseModelMatrix);
    }

    status_t Node::setRotation(const vec3f& rotation, ERotationType rotationType)
    {
        const status_t status = impl.setRotation(rotation, rotationType);
        LOG_HL_CLIENT_API4(status, rotation.x, rotation.y, rotation.z, rotationType);
        return status;
    }

    status_t Node::setRotation(const quat& rotation)
    {
        const status_t status = impl.setRotation(rotation);
        LOG_HL_CLIENT_API4(status, rotation.w, rotation.x, rotation.y, rotation.z);
        return status;
    }

    ramses::ERotationType Node::getRotationType() const
    {
        return impl.getRotationType();
    }

    ramses::status_t Node::getRotation(vec3f& rotation) const
    {
        return impl.getRotation(rotation);
    }

    ramses::status_t Node::getRotation(quat& rotation) const
    {
        return impl.getRotation(rotation);
    }

    ramses::status_t Node::translate(const vec3f& translation)
    {
        const status_t status = impl.translate(translation);
        LOG_HL_CLIENT_API3(status, translation.x, translation.y, translation.z);
        return status;
    }

    ramses::status_t Node::setTranslation(const vec3f& translation)
    {
        const status_t status = impl.setTranslation(translation);
        LOG_HL_CLIENT_API3(status, translation.x, translation.y, translation.z);
        return status;
    }

    ramses::status_t Node::getTranslation(vec3f& translation) const
    {
        return impl.getTranslation(translation);
    }

    ramses::status_t Node::scale(const vec3f& scaling)
    {
        const status_t status = impl.scale(scaling);
        LOG_HL_CLIENT_API3(status, scaling.x, scaling.y, scaling.z);
        return status;
    }

    ramses::status_t Node::setScaling(const vec3f& scaling)
    {
        const status_t status = impl.setScaling(scaling);
        LOG_HL_CLIENT_API3(status, scaling.x, scaling.y, scaling.z);
        return status;
    }

    ramses::status_t Node::getScaling(vec3f& scaling) const
    {
        return impl.getScaling(scaling);
    }

    ramses::status_t Node::setVisibility(EVisibilityMode mode)
    {
        const status_t status = impl.setVisibility(mode);
        LOG_HL_CLIENT_API1(status, VisibilityModeUtils::ToString(mode));
        return status;
    }

    EVisibilityMode Node::getVisibility() const
    {
        return impl.getVisibility();
    }
}
