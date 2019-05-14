//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NODEIMPL_H
#define RAMSES_NODEIMPL_H

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "Collections/Vector.h"

namespace ramses
{
    class SceneImpl;
    class Node;

    class NodeImpl : public SceneObjectImpl
    {
    public:
        NodeImpl(SceneImpl& scene, ERamsesObjectType type, const char* nodeName);
        virtual ~NodeImpl();

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;

        status_t        addChild(NodeImpl& childNode);
        status_t        removeChild(NodeImpl& node);
        status_t        removeParent();
        status_t        setParent(NodeImpl& parentNode);
        status_t        removeAllChildren();

        bool            hasChild() const;
        uint32_t        getChildCount() const;
        Node*           getChild(uint32_t index);
        const Node*     getChild(uint32_t index) const;

        NodeImpl&       getChildImpl(uint32_t index);
        const NodeImpl& getChildImpl(uint32_t index) const;

        bool            hasParent() const;
        Node*           getParent();
        const Node*     getParent() const;
        NodeImpl*       getParentImpl();
        const NodeImpl* getParentImpl() const;

        status_t        getModelMatrix(float(&modelMatrix)[16]) const;
        status_t        getInverseModelMatrix(float(&inverseModelMatrix)[16]) const;

        status_t translate(float x, float y, float z);
        status_t setTranslation(float x, float y, float z);
        status_t getTranslation(float& x, float& y, float& z) const;
        status_t rotate(float x, float y, float z);
        status_t setRotation(float x, float y, float z);
        status_t getRotation(float& x, float& y, float& z) const;
        status_t scale(float x, float y, float z);
        status_t setScaling(float x, float y, float z);
        status_t getScaling(float& x, float& y, float& z) const;

        status_t setVisibility(bool visible);
        bool     getVisibility() const;

        void initializeTransform();
        ramses_internal::TransformHandle getTransformHandle() const;

        ramses_internal::SceneId getSceneId() const;
        ramses_internal::NodeHandle getNodeHandle() const;

        void markDirty();
        bool isDirty() const;

    private:
        typedef std::vector<NodeImpl*> NodeVector;

        void removeChildInternally(NodeVector::iterator childIt);

        ramses_internal::NodeHandle m_nodeHandle;

        NodeVector m_children;
        NodeImpl* m_parent;

        ramses_internal::TransformHandle m_transformHandle;

        //The actual visibility
        bool m_visibility;
    };
}

#endif
