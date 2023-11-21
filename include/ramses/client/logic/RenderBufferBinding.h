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
    class RenderBuffer;
}

namespace ramses::internal
{
    class RenderBufferBindingImpl;
}

namespace ramses
{
    /**
     * The #RenderBufferBinding binds to a Ramses object instance of #ramses::RenderBuffer and exposes a set of input properties
     * allowing to change some parameters of the #ramses::RenderBuffer.
     *
     * Important note: #ramses::RenderBuffer (unlike most other scene objects that can be bound to logic node network) is a static object
     * in the sense that once it is allocated on Ramses renderer side it cannot be changed. This means that #RenderBufferBinding can be
     * effectively used to change properties only BEFORE the render buffer is allocated on renderer (concretely before the scene
     * is in state #ramses::RendererSceneState::Ready). Note that changing its properties after this moment will not produce any API
     * or runtime errors but renderer will ignore them and log an error. It is recommended to study the RenderBufferBinding example
     * before using it.
     *
     * #RenderBufferBinding has these input properties:
     *   'width'  (type int32)
     *       - binds to RenderBuffer's resolution width, overwriting the one which was used when creating it.
     *   'height'  (type int32)
     *       - binds to RenderBuffer's resolution height, overwriting the one which was used when creating it.
     *   'sampleCount'  (type int32)
     *       - binds to RenderBuffer's sample count used for multisampling, overwriting the one which was used when creating it.
     *
     * The initial values of the input properties are taken from the bound #ramses::RenderBuffer.
     *
     * The #RenderBufferBinding class has no output properties (thus #ramses::LogicNode::getOutputs() will return nullptr) because
     * the outputs are implicitly forwarded to the bound #ramses::RenderBuffer.
     *
     * The changes via binding objects are applied to the bound object right away when calling #ramses::LogicEngine::update,
     * however keep in mind that Ramses has a mechanism for bundling scene changes and applying them at once using #ramses::Scene::flush,
     * so the changes will be applied all the way only after calling this method on the scene.
     *
     * Possible errors:
     * - #ramses::LogicEngine::update will fail to update #RenderBufferBinding if:
     *   - any input negative
     *   - 'width' or 'height' is zero
     * - #ramses::Scene::flush can fail as a result of changing #ramses::RenderBuffer parameters in #ramses::Scene:
     *   - if #ramses::RenderBuffer is used in a #ramses::RenderTarget and becomes not compatible (different size or sample count) with other buffers
     *     in that render target
     *   - if #ramses::RenderBuffer is used as source/destination in a #ramses::BlitPass and becomes out of bounds for the pass's blitting region
     * - renderer will produce an error log if a #ramses::RenderBuffer change comes after it was already allocated (see above for details)
     */
    class RAMSES_API RenderBufferBinding : public RamsesBinding
    {
    public:
        /**
        * Returns the bound #ramses::RenderBuffer.
        *
        * @return the bound #ramses::RenderBuffer
        */
        [[nodiscard]] const ramses::RenderBuffer& getRenderBuffer() const;

        /**
        * Returns the bound #ramses::RenderBuffer.
        *
        * @return the bound #ramses::RenderBuffer
        */
        [[nodiscard]] ramses::RenderBuffer& getRenderBuffer();

        /**
         * Get the internal data for implementation specifics of RenderBufferBinding.
         */
        [[nodiscard]] internal::RenderBufferBindingImpl& impl();

        /**
         * Get the internal data for implementation specifics of RenderBufferBinding.
         */
        [[nodiscard]] const internal::RenderBufferBindingImpl& impl() const;

    protected:
        /**
        * Constructor of RenderBufferBinding. User is not supposed to call this - RenderBufferBindings are created by other factory classes
        *
        * @param impl implementation details of the RenderBufferBinding
        */
        explicit RenderBufferBinding(std::unique_ptr<internal::RenderBufferBindingImpl> impl) noexcept;

        /**
         * Implementation detail of RenderBufferBinding
         */
        internal::RenderBufferBindingImpl& m_renderBufferBinding;

        friend class internal::ApiObjects;
    };
}
