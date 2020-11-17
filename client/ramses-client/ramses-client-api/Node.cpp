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

    status_t Node::getModelMatrix(float(&modelMatrix)[16]) const
    {
        return impl.getModelMatrix(modelMatrix);
    }

    status_t Node::getInverseModelMatrix(float(&inverseModelMatrix)[16]) const
    {
        return impl.getInverseModelMatrix(inverseModelMatrix);
    }

    ramses::status_t Node::rotate(float x, float y, float z)
    {
        const status_t status = impl.rotate(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    ramses::status_t Node::setRotation(float x, float y, float z)
    {
        const status_t status = impl.setRotation(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    status_t Node::setRotation(float x, float y, float z, ERotationConvention rotationConvention)
    {
        const status_t status = impl.setRotation(x, y, z, rotationConvention);
        LOG_HL_CLIENT_API4(status, x, y, z, rotationConvention);
        return status;
    }

    ramses::status_t Node::getRotation(float& x, float& y, float& z) const
    {
        return impl.getRotation(x, y, z);
    }

    ramses::status_t Node::getRotation(float& x, float& y, float& z, ERotationConvention& rotationConvention) const
    {
        return impl.getRotation(x, y, z, rotationConvention);
    }

    ramses::status_t Node::translate(float x, float y, float z)
    {
        const status_t status = impl.translate(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    ramses::status_t Node::setTranslation(float x, float y, float z)
    {
        const status_t status = impl.setTranslation(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    ramses::status_t Node::getTranslation(float& x, float& y, float& z) const
    {
        return impl.getTranslation(x, y, z);
    }

    ramses::status_t Node::scale(float x, float y, float z)
    {
        const status_t status = impl.scale(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    ramses::status_t Node::setScaling(float x, float y, float z)
    {
        const status_t status = impl.setScaling(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    ramses::status_t Node::getScaling(float& x, float& y, float& z) const
    {
        return impl.getScaling(x, y, z);
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
