//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// internal
#include "SceneObjectImpl.h"
#include "ramses/framework/EVisibilityMode.h"
#include "ramses/framework/ERotationType.h"
#include "impl/DataTypesImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/ERotationType.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

#include <string_view>

namespace ramses
{
    class Node;
}

namespace ramses::internal
{
    class SceneImpl;

    class NodeImpl : public SceneObjectImpl
    {
    public:
        NodeImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view nodeName);
        ~NodeImpl() override;

        void initializeFrameworkData();
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;

        bool addChild(NodeImpl& childNode);
        bool removeChild(NodeImpl& node);
        bool removeParent();
        bool setParent(NodeImpl& parentNode);
        bool removeAllChildren();

        [[nodiscard]] bool            hasChild() const;
        [[nodiscard]] size_t          getChildCount() const;
        [[nodiscard]] Node*           getChild(size_t index);
        [[nodiscard]] const Node*     getChild(size_t index) const;

        [[nodiscard]] NodeImpl&       getChildImpl(size_t index);
        [[nodiscard]] const NodeImpl& getChildImpl(size_t index) const;

        [[nodiscard]] bool            hasParent() const;
        [[nodiscard]] Node*           getParent();
        [[nodiscard]] const Node*     getParent() const;
        [[nodiscard]] NodeImpl*       getParentImpl();
        [[nodiscard]] const NodeImpl* getParentImpl() const;

        bool getModelMatrix(glm::mat4x4& modelMatrix) const;
        bool getInverseModelMatrix(glm::mat4x4& inverseModelMatrix) const;

        bool translate(const vec3f& translation);
        bool setTranslation(const vec3f& translation);
        bool getTranslation(vec3f& translation) const;
        bool setRotation(const vec3f& rotation, ERotationType rotationType);
        [[nodiscard]] ERotationType getRotationType() const;
        bool getRotation(vec3f& rotation) const;
        bool setRotation(const quat& rotation);
        bool getRotation(quat& rotation) const;
        bool scale(const vec3f& scaling);
        bool setScaling(const vec3f& scaling);
        bool getScaling(vec3f& scaling) const;

        bool setVisibility(EVisibilityMode mode);
        [[nodiscard]] EVisibilityMode getVisibility() const;

        void initializeTransform();
        [[nodiscard]] ramses::internal::TransformHandle getTransformHandle() const;

        [[nodiscard]] ramses::internal::SceneId getSceneId() const;
        [[nodiscard]] ramses::internal::NodeHandle getNodeHandle() const;

        void markDirty();
        [[nodiscard]] bool isDirty() const;

    private:
        using NodeVector = std::vector<NodeImpl *>;

        void removeChildInternally(NodeVector::iterator childIt);
        bool setRotationInternal(glm::vec4&& rotation, ERotationType rotationType);

        ramses::internal::NodeHandle m_nodeHandle;

        NodeVector m_children;
        NodeImpl* m_parent;

        ramses::internal::TransformHandle m_transformHandle;

        //The actual visibility
        EVisibilityMode m_visibilityMode;
    };
}
