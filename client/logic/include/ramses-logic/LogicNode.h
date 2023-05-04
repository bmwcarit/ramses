//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicObject.h"

#include <memory>

namespace ramses::internal
{
    class LogicNodeImpl;
}

namespace ramses
{
    class Property;

    /**
     * A base class for multiple logic classes which provides a unified interface to their
     * inputs and outputs. Some subclasses don't have inputs or outputs - in that case the
     * #getInputs or #getOutputs methods respectively will return nullptr. Some subclasses,
     * like the #ramses::RamsesAppearanceBinding, will have their inputs depending on their
     * current state (in this example the GLSL uniforms of the shader to which the bound ramses
     * Appearance belongs). In those cases, #getInputs()/#getOutputs() will return a #ramses::Property
     * which represents an empty struct (type Struct, but no child properties).
     */
    class LogicNode : public LogicObject
    {
    public:
        /**
         * Returns a property of type Struct which holds the inputs of the #LogicNode.
         *
         * Returns the root Property of the LogicNode which contains potentially
         * nested list of properties. Calling #ramses#Property#getName() on the returned object will always return ""
         * regardless of the name used in the scripts. This applies only to the root input node, rest of the nodes
         * in the tree structure follow standard behavior.
         * The properties are different for the classes which derive from #LogicNode. Look at the documentation
         * of each derived class for more information on the properties.
         * @return a tree like structure with the inputs of the #LogicNode
         */
        [[nodiscard]] RAMSES_API Property* getInputs();

        /**
         * @copydoc getInputs()
         */
        [[nodiscard]] RAMSES_API const Property* getInputs() const;

        /**
         * Returns a property of type Struct which holds the outputs of the #LogicNode
         *
         * Returns the root Property of the LogicNode which contains potentially
         * nested list of properties. Calling #ramses#Property#getName() on the returned object will always return ""
         * regardless of the name used in the scripts. This applies only to the root output node, rest of the nodes
         * in the tree structure follow standard behavior.
         * The properties are different for the classes which derive from #LogicNode. Look at the documentation
         * of each derived class for more information on the properties.
         * @return a tree like structure with the outputs of the LogicNode
         */
        [[nodiscard]] RAMSES_API const Property* getOutputs() const;

        /**
         * Implementation detail of LogicNode
         */
        internal::LogicNodeImpl& m_impl;

    protected:
        /**
         * Constructor of LogicNode. User is not supposed to call this - LogcNodes are created by subclasses
         *
         * @param impl implementation details of the LogicNode
         */
        explicit LogicNode(std::unique_ptr<internal::LogicNodeImpl> impl) noexcept;
    };
}
