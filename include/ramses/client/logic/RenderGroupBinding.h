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
    class RenderGroup;
}

namespace ramses::internal
{
    class RenderGroupBindingImpl;
}

namespace ramses
{
    /**
     * #RenderGroupBinding binds to a Ramses object instance of ramses::RenderGroup.
     * #RenderGroupBinding allows controlling the rendering order of selected elements
     * contained in the ramses::RenderGroup - these can be ramses::MeshNode and other (nested) ramses::RenderGroup objects.
     *
     * #RenderGroupBinding is initialized with a ramses::RenderGroup and a set of elements to expose for render order
     * control through the binding's input properties. Not all the elements contained in the ramses::RenderGroup have to be provided,
     * the binding can be used to control only a subset of the elements in the render group. The set of provided elements is then static
     * for the lifetime of the #RenderGroupBinding.
     *
     * It is not possible to add or remove elements from the bound ramses::RenderGroup using this binding
     * and this binding will not reflect changes made to the ramses::RenderGroup via its Ramses API.
     * Therefore it is important to keep the binding valid with respect to the ramses::RenderGroup it is bound to, for example:
     *   - An element is added to the bound ramses::RenderGroup via Ramses API - the binding with its elements is valid
     *     and can be used but the newly added element will not be known to it (cannot control its render order).
     *   - An element is removed from the bound ramses::RenderGroup via Ramses API - if the removed element was part of the provided
     *     initial set at construction of the binding, the binding is invalidated and should not be used because any attempt to change
     *     render order of the removed element will result in update fail.
     * If #RenderGroupBinding gets invalidated it needs to be destroyed and recreated with a valid set of elements contained in the render group.
     * In this case property links (if any) get destroyed with the binding and need to be re-created.
     *
     * When #RenderGroupBinding is created using #ramses::LogicEngine::createRenderGroupBinding it will create
     * property inputs based on the provided set of elements (see #ramses::RenderGroupBindingElements):
     *   'renderOrders' (type struct) - contains all the elements provided as children:
     *      [element1Name] (type int32) - binds to render order of that element within the bound ramses::RenderGroup
     *      ...
     *      [elementNName] (type int32)
     * The property name for each element can be specified when adding the element using #ramses::RenderGroupBindingElements::addElement.
     * Note that the order of the render order input properties exactly matches the order of adding elements
     * via #ramses::RenderGroupBindingElements::addElement.
     *
     * The render order is directly forwarded to Ramses and follows Ramses behavior of ordering,
     * i.e. element with lower render order value gets rendered before element with higher value.
     * If two elements have same number (default) their render order is not defined and the Ramses renderer decides their order
     * based on its optimization criteria, it is therefore recommended to explicitly specify render order only if needed.
     *
     * The initial render order values of the input properties are taken from the bound ramses::RenderGroup's elements provided during construction.
     *
     * The #RenderGroupBinding class has no output properties (thus #ramses::LogicNode::getOutputs() will return nullptr) because
     * the outputs are implicitly forwarded to the bound ramses::RenderGroup.
     *
     * The changes via binding objects are applied to the bound object right away when calling ramses::LogicEngine::update(),
     * however keep in mind that Ramses has a mechanism for bundling scene changes and applying them at once using ramses::Scene::flush,
     * so the changes will be applied all the way only after calling this method on the scene.
     * @ingroup LogicAPI
     */
    class RAMSES_API RenderGroupBinding : public RamsesBinding
    {
    public:
        /**
        * Returns the bound ramses render group.
        *
        * @return the bound ramses render group
        */
        [[nodiscard]] const ramses::RenderGroup& getRamsesRenderGroup() const;

        /**
        * Returns the bound ramses render group.
        *
        * @return the bound ramses render group
        */
        [[nodiscard]] ramses::RenderGroup& getRamsesRenderGroup();

        /**
         * Get the internal data for implementation specifics of RenderGroupBinding.
         */
        [[nodiscard]] internal::RenderGroupBindingImpl& impl();

        /**
         * Get the internal data for implementation specifics of RenderGroupBinding.
         */
        [[nodiscard]] const internal::RenderGroupBindingImpl& impl() const;

    protected:
        /**
        * Constructor of RenderGroupBinding. User is not supposed to call this - RenderGroupBindings are created by other factory classes
        *
        * @param impl implementation details of the RenderGroupBinding
        */
        explicit RenderGroupBinding(std::unique_ptr<internal::RenderGroupBindingImpl> impl) noexcept;

        /**
         * Implementation detail of RenderGroupBinding
         */
        internal::RenderGroupBindingImpl& m_renderGroupBinding;

        friend class internal::ApiObjects;
    };
}
