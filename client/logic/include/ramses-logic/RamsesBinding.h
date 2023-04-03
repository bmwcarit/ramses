//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicNode.h"

namespace rlogic::internal
{
    class RamsesBindingImpl;
}

namespace rlogic
{
    /**
     * The RamsesBinding is a shared base class for bindings to Ramses objects.
     * For details on each type of binding, look at the derived classes.
     */
    class RamsesBinding : public LogicNode
    {
    public:
        /**
         * Constructor of RamsesBinding. User is not supposed to call this - RamsesBinding are created by other factory classes
         *
         * @param impl implementation details of the RamsesBinding
         */
        explicit RamsesBinding(std::unique_ptr<internal::RamsesBindingImpl> impl) noexcept;

        /**
         * Destructor of RamsesBinding.
         */
        ~RamsesBinding() noexcept override = default;

        /**
         * Copy Constructor of RamsesBinding is deleted because RamsesBindings are not supposed to be copied
         *
         * @param other RamsesBindings to copy from
         */
        RamsesBinding(const RamsesBinding& other) = delete;

        /**
         * Move Constructor of RamsesBinding is deleted because RamsesBindings are not supposed to be moved
         *
         * @param other RamsesBindings to move from
         */
        RamsesBinding(RamsesBinding&& other) = delete;

        /**
         * Assignment operator of RamsesBinding is deleted because RamsesBindings are not supposed to be copied
         *
         * @param other RamsesBindings to assign from
         */
        RamsesBinding& operator=(const RamsesBinding& other) = delete;

        /**
         * Move assignment operator of RamsesBinding is deleted because RamsesBindings are not supposed to be moved
         *
         * @param other RamsesBindings to assign from
         */
        RamsesBinding& operator=(RamsesBinding&& other) = delete;
    };
}
