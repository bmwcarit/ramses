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
#include "ramses-client-api/EVisibilityMode.h"
#include "ramses-client-api/ERotationType.h"
#include "DataTypesImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/ERotationType.h"
#include "Collections/Vector.h"

#include <string_view>

namespace ramses
{
    class SceneImpl;
    class Node;

    class NodeImpl : public SceneObjectImpl
    {
    public:
        NodeImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view nodeName);
        ~NodeImpl() override;

        void             initializeFrameworkData();
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;

        status_t        addChild(NodeImpl& childNode);
        status_t        removeChild(NodeImpl& node);
        status_t        removeParent();
        status_t        setParent(NodeImpl& parentNode);
        status_t        removeAllChildren();

        bool            hasChild() const;
        size_t          getChildCount() const;
        Node*           getChild(size_t index);
        const Node*     getChild(size_t index) const;

        NodeImpl&       getChildImpl(size_t index);
        const NodeImpl& getChildImpl(size_t index) const;

        bool            hasParent() const;
        Node*           getParent();
        const Node*     getParent() const;
        NodeImpl*       getParentImpl();
        const NodeImpl* getParentImpl() const;

        status_t        getModelMatrix(glm::mat4x4& modelMatrix) const;
        status_t        getInverseModelMatrix(glm::mat4x4& inverseModelMatrix) const;

        status_t translate(const vec3f& translation);
        status_t setTranslation(const vec3f& translation);
        status_t getTranslation(vec3f& translation) const;
        status_t setRotation(const vec3f& rotation, ERotationType rotationType);
        ERotationType getRotationType() const;
        status_t getRotation(vec3f& rotation) const;
        status_t setRotation(const quat& rotation);
        status_t getRotation(quat& rotation) const;
        status_t scale(const vec3f& scaling);
        status_t setScaling(const vec3f& scaling);
        status_t getScaling(vec3f& scaling) const;

        status_t setVisibility(EVisibilityMode mode);
        EVisibilityMode getVisibility() const;

        void initializeTransform();
        ramses_internal::TransformHandle getTransformHandle() const;

        ramses_internal::SceneId getSceneId() const;
        ramses_internal::NodeHandle getNodeHandle() const;

        void markDirty();
        bool isDirty() const;

    private:
        using NodeVector = std::vector<NodeImpl *>;

        void removeChildInternally(NodeVector::iterator childIt);
        status_t setRotationInternal(glm::vec4&& rotation, ramses_internal::ERotationType rotationType);

        ramses_internal::NodeHandle m_nodeHandle;

        NodeVector m_children;
        NodeImpl* m_parent;

        ramses_internal::TransformHandle m_transformHandle;

        //The actual visibility
        EVisibilityMode m_visibilityMode;
    };
}

#endif
