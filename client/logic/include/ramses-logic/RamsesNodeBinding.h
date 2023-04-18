//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-client-api/ERotationType.h"
#include "ramses-logic/RamsesBinding.h"

#include <memory>

namespace ramses
{
    class Node;
}

namespace rlogic::internal
{
    class RamsesNodeBindingImpl;
}

namespace rlogic
{
    /**
     * The RamsesNodeBinding is a #rlogic::RamsesBinding which allows manipulation of a Ramses node.
     * RamsesNodeBinding can be created using #rlogic::LogicEngine::createRamsesNodeBinding.
     *
     * The RamsesNodeBinding has a fixed set of inputs which correspond to the properties of a ramses::Node:
     *   'visibility' (type bool)     - binds to node's visibility mode to switch between visible and invisible,
     *                                  see also 'enabled' input for more options
     *   'rotation' (type vec3f, or vec4f in case of quaternion) - binds to node's rotation,
     *                                  a vec3f represents the Euler angles v[0]:x, v[1]:y, v[2]:z
     *                                  a vec4f represents a quaternion with v[0]:x, v[1]:y, v[2]:z containing the quaternion's vector component
     *                                  and v[3]:w the quaternion's scalar component
     *   'translation' (type vec3f)   - binds to node's translation
     *   'scaling' (type vec3f)       - binds to node's scaling
     *   'enabled' (type bool)        - binds to node's visibility mode to be able to switch node to off mode
     *                                  (see ramses::EVisibilityMode). When 'enabled' is true (default)
     *                                  the visibility mode is determined by the 'visibility' input above.
     *                                  When 'enabled' is false, the visibility mode is Off,
     *                                  regardless of the 'visibility' input.
     *
     * The default values of the input properties are taken from the bound ramses::Node provided during construction.
     * This also applies for rotations, if the ramses::ERotationType values of the
     * ramses node match (both are either Quaternion or Euler with the same axis ordering). Otherwise a warning is
     * issued and the rotation values are set to 0.
     *
     * The RamsesNodeBinding class has no output properties (thus getOutputs() will return nullptr) because
     * the outputs are implicitly the properties of the bound Ramses node.
     *
     * \rst
    .. note::

        Not all states of the #ramses::Node must be managed via #RamsesNodeBinding, input properties which are not modified
        via #RamsesNodeBinding (i.e. user never set a value explicitly nor linked the input) will not alter the corresponding Ramses object
        state. This allows the user code to control some states via Ramses logic nodes and some directly using Ramses object.

     \endrst
     *
     * The changes via binding objects are applied to the bound object right away when calling rlogic::LogicEngine::update(),
     * however keep in mind that Ramses has a mechanism for bundling scene changes and applying them at once using ramses::Scene::flush,
     * so the changes will be applied all the way only after calling this method on the scene.
     */
    class RamsesNodeBinding : public RamsesBinding
    {
    public:
        /**
        * Returns the bound ramses node.
        *
        * @return the bound ramses node
        */
        [[nodiscard]] RAMSES_API ramses::Node& getRamsesNode() const;

        /**
        * Returns the statically configured rotation type for the node rotation property.
        *
        * @return the currently used rotation type
        */
        [[nodiscard]] RAMSES_API ramses::ERotationType getRotationType() const;

        /**
        * Constructor of RamsesNodeBinding. User is not supposed to call this - RamsesNodeBindings are created by other factory classes
        *
        * @param impl implementation details of the RamsesNodeBinding
        */
        explicit RamsesNodeBinding(std::unique_ptr<internal::RamsesNodeBindingImpl> impl) noexcept;

        /**
         * Destructor of RamsesNodeBinding.
         */
        ~RamsesNodeBinding() noexcept override;

        /**
         * Copy Constructor of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be copied
         *
         * @param other RamsesNodeBindings to copy from
         */
        RamsesNodeBinding(const RamsesNodeBinding& other) = delete;

        /**
         * Move Constructor of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be moved
         *
         * @param other RamsesNodeBindings to move from
         */
        RamsesNodeBinding(RamsesNodeBinding&& other) = delete;

        /**
         * Assignment operator of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be copied
         *
         * @param other RamsesNodeBindings to assign from
         */
        RamsesNodeBinding& operator=(const RamsesNodeBinding& other) = delete;

        /**
         * Move assignment operator of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be moved
         *
         * @param other RamsesNodeBindings to assign from
         */
        RamsesNodeBinding& operator=(RamsesNodeBinding&& other) = delete;

        /**
         * Implementation detail of RamsesNodeBinding
         */
        internal::RamsesNodeBindingImpl& m_nodeBinding;
    };
}
