//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-logic/RamsesBinding.h"

#include <memory>

namespace ramses
{
    class Appearance;
}

namespace ramses::internal
{
    class RamsesAppearanceBindingImpl;
}

namespace ramses
{
    /**
     * The RamsesAppearanceBinding is a type of #ramses::RamsesBinding which allows the #ramses::LogicEngine to control instances
     * of ramses::Appearance. RamsesAppearanceBinding's can be created with #ramses::LogicEngine::createRamsesAppearanceBinding.
     *
     * The #RamsesAppearanceBinding has a static link to a ramses::Appearance. After creation, #ramses::LogicNode::getInputs will
     * return a struct property with children equivalent to the uniform inputs of the provided ramses Appearance.
     *
     * Since the RamsesAppearanceBinding derives from #ramses::RamsesBinding, it also provides the #ramses::LogicNode::getInputs
     * and #ramses::LogicNode::getOutputs method. For this particular implementation, the methods behave as follows:
     *  - #ramses::LogicNode::getInputs: returns the inputs corresponding to the available shader uniforms of the bound ramses::Appearance
     *  - #ramses::LogicNode::getOutputs: returns always nullptr, because a #RamsesAppearanceBinding does not have outputs,
     *    it implicitly controls the ramses Appearance
     *  - The values of this binding's inputs are initialized to default values (0, 0.0f, etc) and *not* loaded from the values in Ramses
     *
     * All shader uniforms are supported, except the following:
     * - texture samplers of any kind
     * - matrix types (e.g. mat4, mat23 etc.)
     * - any uniform with attached semantics (e.g. display resolution) - see ramses::EEffectUniformSemantic docs
     *
     * Uniform types which are not supported are not available when queried over #ramses::LogicNode::getInputs.
     *
     */
    class RamsesAppearanceBinding : public RamsesBinding
    {
    public:
        /**
         * Returns the bound Ramses Appearance.
         * @return the bound ramses appearance
         */
        [[nodiscard]] RAMSES_API ramses::Appearance& getRamsesAppearance() const;

        /**
         * Implementation detail of RamsesAppearanceBinding
         */
        internal::RamsesAppearanceBindingImpl& m_appearanceBinding;

    protected:
        /**
         * Constructor of RamsesAppearanceBinding. User is not supposed to call this - RamsesAppearanceBinding are created by other factory classes
         *
         * @param impl implementation details of the RamsesAppearanceBinding
         */
        explicit RamsesAppearanceBinding(std::unique_ptr<internal::RamsesAppearanceBindingImpl> impl) noexcept;

        friend class internal::ApiObjects;
    };
}
