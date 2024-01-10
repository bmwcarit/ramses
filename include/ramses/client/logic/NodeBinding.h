//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/framework/ERotationType.h"
#include "ramses/client/logic/RamsesBinding.h"

#include <memory>

namespace ramses
{
    class Node;
}

namespace ramses::internal
{
    class NodeBindingImpl;
}

namespace ramses
{
    /**
     * The NodeBinding is a #ramses::RamsesBinding which allows manipulation of a Ramses node.
     * NodeBinding can be created using #ramses::LogicEngine::createNodeBinding.
     *
     * The NodeBinding has a fixed set of inputs which correspond to the properties of a ramses::Node:
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
     * This also applies to rotations, if the ramses::ERotationType values of the
     * ramses node match (both are either Quaternion or Euler with the same axis ordering). Otherwise a warning is
     * issued and the rotation values are set to 0.
     *
     * The NodeBinding class has no output properties (thus getOutputs() will return nullptr) because
     * the outputs are implicitly the properties of the bound Ramses node.
     *
     * \rst
    .. note::

        Not all states of the #ramses::Node must be managed via #NodeBinding, input properties which are not modified
        via #NodeBinding (i.e. user never set a value explicitly nor linked the input) will not alter the corresponding Ramses object
        state. This allows the user code to control some states via Ramses logic nodes and some directly using Ramses object.

     \endrst
     *
     * The changes via binding objects are applied to the bound object right away when calling ramses::LogicEngine::update(),
     * however keep in mind that Ramses has a mechanism for bundling scene changes and applying them at once using ramses::Scene::flush,
     * so the changes will be applied all the way only after calling this method on the scene.
     * @ingroup LogicAPI
     */
    class RAMSES_API NodeBinding : public RamsesBinding
    {
    public:
        /**
        * Returns the bound ramses node.
        *
        * @return the bound ramses node
        */
        [[nodiscard]] ramses::Node& getRamsesNode() const;

        /**
        * Returns the statically configured rotation type for the node rotation property.
        *
        * @return the currently used rotation type
        */
        [[nodiscard]] ramses::ERotationType getRotationType() const;

        /**
         * Get the internal data for implementation specifics of NodeBinding.
         */
        [[nodiscard]] internal::NodeBindingImpl& impl();

        /**
         * Get the internal data for implementation specifics of NodeBinding.
         */
        [[nodiscard]] const internal::NodeBindingImpl& impl() const;

    protected:
        /**
        * Constructor of NodeBinding. User is not supposed to call this - NodeBindings are created by other factory classes
        *
        * @param impl implementation details of the NodeBinding
        */
        explicit NodeBinding(std::unique_ptr<internal::NodeBindingImpl> impl) noexcept;

        /**
         * Implementation detail of NodeBinding
         */
        internal::NodeBindingImpl& m_nodeBinding;

        friend class internal::ApiObjects;
    };
}
