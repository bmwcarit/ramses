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
#include "ramses-client-api/ERotationConvention.h"

namespace ramses
{
    /**
     * @brief The Node is the base class of all nodes and provides
     * scene graph functionality which propagates to its children.
     */
    class RAMSES_API Node : public SceneObject
    {
    public:
        /**
         * @brief Returns if node has at least one child Node.
         *
         * @return true if this Node has at least one child Node, false otherwise.
         */
        bool hasChild() const;

        /**
        * @brief Gets the number of child Nodes of this node.
        *
        * @return Number of child Nodes of this Node.
        */
        uint32_t getChildCount() const;

        /**
        * @brief Gets child node at provided index.
        *
        * @param[in] index Index of the child Node to be returned. Index range is [0..getChildCount()-1].
        *
        * @return Pointer to Node, if child at index exists.
        * @return nullptr if child at index does not exist.
        */
        Node* getChild(uint32_t index);
        /** @copydoc getChild(uint32_t) */
        const Node* getChild(uint32_t index) const;

        /**
        * @brief Adds child Node to this node.
        *
        * @param[in] node Node to be set as child for this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addChild(Node& node);

        /**
        * @brief Removes a child Node from this node.
        *
        * @param[in] node Node to be removed as child from this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeChild(Node& node);

        /**
        * @brief Removes all child Nodes from this node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeAllChildren();

        /**
        * @brief Returns if Node has a parent Node.
        *
        * @return true if Node has a parent Node, false otherwise.
        */
        bool hasParent() const;

        /**
        * @brief Gets parent Node of this Node.
        *
        * @return Pointer to parent Node, if parent exists, nullptr otherwise.
        */
        Node* getParent();
        /** @copydoc getParent() */
        const Node* getParent() const;

        /**
        * @brief Sets parent Node for this node.
        *
        * @param[in] node Node to be set as parent for this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setParent(Node& node);

        /**
        * @brief Removes the parent Node from this Node.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeParent();

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
        status_t getModelMatrix(float(&modelMatrix)[16]) const;

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
        status_t getInverseModelMatrix(float(&inverseModelMatrix)[16]) const;

        /**
        * @brief Rotates in all three directions with the given values without affecting
        *        the currently set rotation convention.
        *
        *        This function can be used only if #ramses::Node::setRotation(float,float,float) is used to set
        *        node rotation, which implicitly uses left-handed Euler ZYX rotation convention, otherwise it will fail.
        *
        * @deprecated This function is deprecated and will be removed in one of the next
        *             major releases. It is encouraged if possible to already migrate to using
        *             #ramses::Node::setRotation(float,float,float,ERotationConvention) and #ramses::Node::getRotation(float&,float&,float&,ERotationConvention&)const.
        *
        * @param[in] x Value in degrees which is added to the current rotation around x-axis
        * @param[in] y Value in degrees which is added to the current rotation around y-axis
        * @param[in] z Value in degrees which is added to the current rotation around z-axis
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t rotate(float x, float y, float z);

        /**
        * @brief Sets the absolute rotation in all three directions for left-handed rotation
        *        using Euler ZYX rotation convention. If this function is used to set, then only
        *        #ramses::Node::getRotation(float&,float&,float&)const can be used to get node rotation, which
        *        implicitly uses left-handed Euler ZYX rotation convention.
        *
        * @deprecated This function is deprecated and will be removed in one of the next
        *             major releases. It is encouraged if possible to already migrate to using
        *             #ramses::Node::setRotation(float,float,float,ERotationConvention) and #ramses::Node::getRotation(float&,float&,float&,ERotationConvention&)const.
        *
        * @param[in] x The value in degrees for the rotation around x-axis.
        * @param[in] y The value in degrees for the rotation around y-axis.
        * @param[in] z The value in degrees for the rotation around z-axis.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRotation(float x, float y, float z);

        /**
        * @brief Sets the absolute rotation in all three directions for right-handed rotation using the chosen Euler
        *        angles rotation convention. If this function is used to set, then only
        *        #ramses::Node::getRotation(float&,float&,float&,ERotationConvention&)const can be used to get node rotation.
        *
        * @param[in] x The value in degrees for the rotation around x-axis in case of Tait-Bryan conventions, for Proper Euler conventions specifies
                       the rotation angle for the first angle in the convention name.
        * @param[in] y The value in degrees for the rotation around y-axis in case of Tait-Bryan conventions, for Proper Euler conventions specifies
                       the rotation angle for the second angle in the convention name.
        * @param[in] z The value in degrees for the rotation around z-axis in case of Tait-Bryan conventions, for Proper Euler conventions specifies
                       the rotation angle for the third angle in the convention name.
        * @param[in] rotationConvention The rotation convention to use for calculation of rotation matrix.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRotation(float x, float y, float z, ERotationConvention rotationConvention);

        /**
        * @brief Retrieves the absolute rotation for left-handed rotation using
        *        Euler ZYX rotation convention.
        *
        *        This function can be used only if #ramses::Node::setRotation(float,float,float) is used to set
        *        node rotation, which implicitly uses left-handed Euler ZYX rotation convention, otherwise it will fail.
        *
        *        Default value is 0 for all components.
        *
        * @deprecated This function is deprecated and will be removed in one of the next
        *             major releases. It is encouraged if possible to already migrate to using
        *             #ramses::Node::setRotation(float,float,float,ERotationConvention) and #ramses::Node::getRotation(float&,float&,float&,ERotationConvention&)const.
        *
        * @param[out] x Current value in degrees on x-axis.
        * @param[out] y Current value in degrees on y-axis.
        * @param[out] z Current value in degrees on z-axis.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getRotation(float& x, float& y, float& z) const;

        /**
        * @brief Retrieves the absolute rotation for right-handed rotation in all three directions and the used Euler
        *        angles rotation convention. This function can be used only
        *        if #ramses::Node::setRotation(float,float,float,ERotationConvention) is used to set node rotation.
        *
        *        Default value is 0 for all components and ERotationConvention::XYZ.
        *
        * @param[out] x Current value in degrees for rotation around x-axis in case of Tait-Bryan conventions, in case of Proper Euler conventions gets
                       the rotation angle for the first angle in the convention name.
        * @param[out] y Current value in degrees for rotation around y-axis in case of Tait-Bryan conventions, in case of Proper Euler conventions gets
                       the rotation angle for the second angle in the convention name.
        * @param[out] z Current value in degrees for rotation around z-axis in case of Tait-Bryan conventions, in case of Proper Euler conventions gets
                       the rotation angle for the third angle in the convention name.
        * @param[out] rotationConvention The rotation convention to use for calculation of rotation matrix.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getRotation(float& x, float& y, float& z, ERotationConvention& rotationConvention) const;

        /**
        * @brief Translates in all three directions with the given values.
        *
        * @param[in] x Float with relative translation from origin on x-axis.
        * @param[in] y Float with relative translation from origin on y-axis.
        * @param[in] z Float with relative translation from origin on z-axis.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t translate(float x, float y, float z);

        /**
        * @brief Sets the absolute translation  the absolute values.
        *
        * @param[in] x Float with absolute translation value in x-axis.
        * @param[in] y Float with absolute translation value in y-axis.
        * @param[in] z Float with absolute translation value in z-axis.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setTranslation(float x, float y, float z);

        /**
        * @brief Retrieves the current absolute translation.
        *
        * @param[out] x Current translation on x-axis.
        * @param[out] y Current translation on y-axis.
        * @param[out] z Current translation on z-axis.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getTranslation(float& x, float& y, float& z) const;

        /**
        * @brief Scales in all three directions with the given values.
        *
        * @param[in] x The relative scaling factor which in x-dimension.
        * @param[in] y The relative scaling factor which in y-dimension.
        * @param[in] z The relative scaling factor which in z-dimension.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t scale(float x, float y, float z);

        /**
        * @brief Sets the absolute scale in all three dimensions.
        *
        * @param[in] x The scaling factor in x-dimension.
        * @param[in] y The scaling factor in y-dimension.
        * @param[in] z The scaling factor in z-dimension.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setScaling(float x, float y, float z);

        /**
        * @brief Retrieves the current absolute scale in all three dimensions.
        *
        * @param[out] x The current scaling in x-dimension.
        * @param[out] y The current scaling in y-dimension.
        * @param[out] z The current scaling in z-dimension.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getScaling(float& x, float& y, float& z) const;

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
        status_t setVisibility(EVisibilityMode mode);

        /**
        * @brief Gets the visibility property of the Node. This is just the visibility
        * state of this node object, NOT the hierarchically accumulated visibility of its parents.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        EVisibilityMode getVisibility() const;

        /**
        * Stores internal data for implementation specifics of Node.
        */
        class NodeImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating Node instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for Node.
        *
        * @param[in] pimpl Internal data for implementation specifics of Node (sink - instance becomes owner)
        */
        explicit Node(NodeImpl& pimpl);

        /**
        * @brief Destructor of the Node
        */
        virtual ~Node();
    };
}

#endif
