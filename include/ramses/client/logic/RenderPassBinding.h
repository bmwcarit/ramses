//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/client/logic/RamsesBinding.h"

namespace ramses
{
    class RenderPass;
}

namespace ramses::internal
{
    class RenderPassBindingImpl;
}

namespace ramses
{
    /**
     * #RenderPassBinding binds to a Ramses object instance like other types derived from
     * #ramses::RamsesBinding, in this case it binds to a ramses::RenderPass.
     * RenderPassBinding can be created using #ramses::LogicEngine::createRenderPassBinding
     * after providing an instance of a ramses::RenderPass.
     *
     * The RenderPassBinding has a fixed set of inputs which correspond to some of the parameters in ramses::RenderPass:
     *   'enabled' (type bool)      - binds to enable/disable state (see ramses::RenderPass::setEnabled)
     *   'renderOrder' (type int32) - binds to render order (see ramses::RenderPass::setRenderOrder)
     *   'clearColor' (type vec4f)  - binds to color used when clearing render pass (see ramses::RenderPass::setClearColor)
     *   'renderOnce' (type bool)   - binds to render once on/off state (see ramses::RenderPass::setRenderOnce)
     *
     * The initial values of the input properties are taken from the bound ramses::RenderPass provided during construction.
     *
     * The #RenderPassBinding class has no output properties (thus #ramses::LogicNode::getOutputs() will return nullptr) because
     * the outputs are implicitly the properties of the bound ramses::RenderPass.
     *
     * \rst
    .. note::

        Not all states of the #ramses::RenderPass must be managed via #RenderPassBinding, input properties which are not modified
        via #RenderPassBinding (i.e. user never set a value explicitly nor linked the input) will not alter the corresponding Ramses object
        state. This allows the user code to control some states via Ramses logic nodes and some directly using Ramses object.

     \endrst
     *
     * The changes via binding objects are applied to the bound object right away when calling ramses::LogicEngine::update(),
     * however keep in mind that Ramses has a mechanism for bundling scene changes and applying them at once using ramses::Scene::flush,
     * so the changes will be applied all the way only after calling this method on the scene.
     * @ingroup LogicAPI
     */
    class RAMSES_API RenderPassBinding : public RamsesBinding
    {
    public:
        /**
        * Returns the bound ramses render pass.
        *
        * @return the bound ramses render pass
        */
        [[nodiscard]] const ramses::RenderPass& getRamsesRenderPass() const;

        /**
        * Returns the bound ramses render pass.
        *
        * @return the bound ramses render pass
        */
        [[nodiscard]] ramses::RenderPass& getRamsesRenderPass();

        /**
         * Get the internal data for implementation specifics of RenderPassBinding.
         */
        [[nodiscard]] internal::RenderPassBindingImpl& impl();

        /**
         * Get the internal data for implementation specifics of RenderPassBinding.
         */
        [[nodiscard]] const internal::RenderPassBindingImpl& impl() const;

    protected:
        /**
        * Constructor of RenderPassBinding. User is not supposed to call this - RenderPassBindings are created by other factory classes
        *
        * @param impl implementation details of the RenderPassBinding
        */
        explicit RenderPassBinding(std::unique_ptr<internal::RenderPassBindingImpl> impl) noexcept;

        /**
         * Implementation detail of RenderPassBinding
         */
        internal::RenderPassBindingImpl& m_renderPassBinding;

        friend class internal::ApiObjects;
    };
}
