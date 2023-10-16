//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/client/logic/RamsesBinding.h"

#include <memory>

namespace ramses
{
    class Appearance;
}

namespace ramses::internal
{
    class AppearanceBindingImpl;
}

namespace ramses
{
    /**
     * The AppearanceBinding is a type of #ramses::RamsesBinding which allows the #ramses::LogicEngine to control instances
     * of ramses::Appearance. AppearanceBinding's can be created with #ramses::LogicEngine::createAppearanceBinding.
     *
     * The #AppearanceBinding has a static link to a ramses::Appearance. After creation, #ramses::LogicNode::getInputs will
     * return a struct property with children equivalent to the uniform inputs of the provided ramses Appearance.
     *
     * Since the AppearanceBinding derives from #ramses::RamsesBinding, it also provides the #ramses::LogicNode::getInputs
     * and #ramses::LogicNode::getOutputs method. For this particular implementation, the methods behave as follows:
     *  - #ramses::LogicNode::getInputs: returns the inputs corresponding to the available shader uniforms of the bound ramses::Appearance
     *  - #ramses::LogicNode::getOutputs: returns always nullptr, because a #AppearanceBinding does not have outputs,
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
    class RAMSES_API AppearanceBinding : public RamsesBinding
    {
    public:
        /**
         * Returns the bound Ramses Appearance.
         * @return the bound ramses appearance
         */
        [[nodiscard]] ramses::Appearance& getRamsesAppearance() const;

        /**
         * Get the internal data for implementation specifics of AppearanceBinding.
         */
        [[nodiscard]] internal::AppearanceBindingImpl& impl();

        /**
         * Get the internal data for implementation specifics of AppearanceBinding.
         */
        [[nodiscard]] const internal::AppearanceBindingImpl& impl() const;

    protected:
        /**
         * Constructor of AppearanceBinding. User is not supposed to call this - AppearanceBinding are created by other factory classes
         *
         * @param impl implementation details of the AppearanceBinding
         */
        explicit AppearanceBinding(std::unique_ptr<internal::AppearanceBindingImpl> impl) noexcept;

        /**
         * Implementation detail of AppearanceBinding
         */
        internal::AppearanceBindingImpl& m_appearanceBinding;

        friend class internal::ApiObjects;
    };
}
