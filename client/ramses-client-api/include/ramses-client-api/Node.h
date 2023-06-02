//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NODE_H
#define RAMSES_NODE_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-client-api/EVisibilityMode.h"
#include "ramses-client-api/ERotationType.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    /**
     * @ingroup CoreAPI
     * @brief The Node is the base class of all nodes and provides
     * scene graph functionality which propagates to its children.
     */
    class Node : public SceneObject
    {
    public:
        /**
         * @brief Returns if node has at least one child Node.
         *
         * @return true if this Node has at least one child Node, false otherwise.
         */
        [[nodiscard]] RAMSES_API bool hasChild() const;

        /**
        * @brief Gets the number of child Nodes of this node.
        *
        * @return Number of child Nodes of this Node.
        */
        [[nodiscard]] RAMSES_API size_t getChildCount() const;

        /**
        * @brief Gets child node at provided index.
        *
        * @param[in] index Index of the child Node to be returned. Index range is [0..getChildCount()-1].
        *
        * @return Pointer to Node, if child at index exists.
        * @return nullptr if child at index does not exist.
        */
        Node* getChild(size_t index);
        /** @copydoc getChild(size_t) */
        [[nodiscard]] RAMSES_API const Node* getChild(size_t index) const;

        /**
        * @brief Adds child Node to this node.
        *
        * @param[in] node Node to be set as child for this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t addChild(Node& node);

        /**
        * @brief Removes a child Node from this node.
        *
        * @param[in] node Node to be removed as child from this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t removeChild(Node& node);

        /**
        * @brief Removes all child Nodes from this node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t removeAllChildren();

        /**
        * @brief Returns if Node has a parent Node.
        *
        * @return true if Node has a parent Node, false otherwise.
        */
        [[nodiscard]] RAMSES_API bool hasParent() const;

        /**
        * @brief Gets parent Node of this Node.
        *
        * @return Pointer to parent Node, if parent exists, nullptr otherwise.
        */
        Node* getParent();
        /** @copydoc getParent() */
        [[nodiscard]] RAMSES_API const Node* getParent() const;

        /**
        * @brief Sets parent Node for this node.
        *
        * @param[in] node Node to be set as parent for this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setParent(Node& node);

        /**
        * @brief Removes the parent Node from this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t removeParent();

        /**
        * @brief Gets model (world in scene space) matrix computed from the scene graph.
        *        Performance note: this call will cause computation of a transformation chain,
        *        however the result is cached therefore subsequent calls will have little to none
        *        performance overhead as long as topology or transformation in the chain does not change.
        *
        * @param[out] modelMatrix Will be filled with the model matrix 4x4 column-major
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getModelMatrix(matrix44f& modelMatrix) const;

        /**
        * @brief Gets inverse model (world in scene space) matrix computed from the scene graph.
        *        Performance note: this call will cause computation of a transformation chain,
        *        however the result is cached therefore subsequent calls will have little to none
        *        performance overhead as long as topology or transformation in the chain does not change.
        *
        * @param[out] inverseModelMatrix Will be filled with the inverse model matrix 4x4 column-major
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getInverseModelMatrix(matrix44f& inverseModelMatrix) const;

        /**
        * @brief Sets the absolute rotation in all three directions for right-handed rotation using the chosen Euler
        *        angles rotation convention. If this function is used to set, then only
        *        #ramses::Node::getRotation(vec3f&)const can be used to get node rotation.
        *        Will return an error if the given rotationType is not an Euler convention.
        *
        * @param[in] rotation Euler angles in degrees
        * @param[in] rotationType The rotation convention to use for calculation of rotation matrix. Default value set to Euler_XYZ rotation convention.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setRotation(const vec3f& rotation, ERotationType rotationType = ERotationType::Euler_XYZ);

        /**
        * @brief Sets the absolute rotation defined by a quaternion.
        *        If this function is used to set, then only #ramses::Node::getRotation(quat&)const can be used to get the node rotation.
        *
        * @param[in] rotation a normalized quaternion
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setRotation(const quat& rotation);

        /**
        * @brief Returns the current rotation type applied to the node
        *        The value is modified by #ramses::Node::setRotation().
        *        Default rotation type is #ramses::ERotationType::Euler_XYZ
        *
        * @return rotation type
        */
        [[nodiscard]] RAMSES_API ERotationType getRotationType() const;

        /**
        * @brief Retrieves the absolute rotation for right-handed rotation in all three directions for the used Euler
        *        angles rotation convention. This function will return an error if no Euler rotation convention is set
        *        (check #ramses::Node::getRotationType() before)
        *
        *        Default value is 0 for all components.
        *
        * @param[out] rotation Euler angles in degrees
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getRotation(vec3f& rotation) const;

        /**
        * @brief Retrieves the absolute rotation defined by a quaternion.
        *        This function will return an error if #ramses::Node::getRotationType() != #ramses::ERotationType::Quaternion
        *        Default value is an identity quaternion: glm::identity<quat>()
        *
        * @param[out] rotation quaternion
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getRotation(quat& rotation) const;

        /**
        * @brief Translates in all three directions with the given values.
        *
        * @param[in] translation relative translation from origin.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t translate(const vec3f& translation);

        /**
        * @brief Sets the absolute translation  the absolute values.
        *
        * @param[in] translation absolute translation value
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setTranslation(const vec3f& translation);

        /**
        * @brief Retrieves the current absolute translation.
        *
        * @param[out] translation Current translation
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getTranslation(vec3f& translation) const;

        /**
        * @brief Scales in all three directions with the given values.
        *
        * @param[in] scaling The relative scaling factor.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t scale(const vec3f& scaling);

        /**
        * @brief Sets the absolute scale in all three dimensions.
        *
        * @param[in] scaling scaling factor.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setScaling(const vec3f& scaling);

        /**
        * @brief Retrieves the current absolute scale in all three dimensions.
        *
        * @param[out] scaling The current scaling factor.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t getScaling(vec3f& scaling) const;

        /**
        * @brief Sets the visibility of the Node.
        *        Visibility of a node determines if a renderable is rendered or not and if
        *        its resources are loaded. See \ref EVisibilityMode for more details.
        *        Those attributes are propagated down to the node's children recursively. A node
        *        can only be rendered, if none of its parents are Invisible or Off, the node's
        *        resources are only loaded if none of its parents are Off.
        *
        * @param[in] mode Visibility mode for this node (default is Visible)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setVisibility(EVisibilityMode mode);

        /**
        * @brief Gets the visibility property of the Node. This is just the visibility
        * state of this node object, NOT the hierarchically accumulated visibility of its parents.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        [[nodiscard]] RAMSES_API EVisibilityMode getVisibility() const;

        /**
        * Stores internal data for implementation specifics of Node.
        */
        class NodeImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating Node instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor for Node.
        *
        * @param[in] impl Internal data for implementation specifics of Node (sink - instance becomes owner)
        */
        explicit Node(std::unique_ptr<NodeImpl> impl);
    };
}

#endif
