//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneObject.h"

namespace ramses::internal
{
    class LogicObjectImpl;
    class ApiObjects;
}

namespace ramses
{
    /**
    * A base class for all rlogic API objects.
    * @ingroup LogicAPI
    */
    class RAMSES_API LogicObject : public SceneObject
    {
    public:
        /**
         * Get the internal data for implementation specifics of #LogicObject.
         */
        [[nodiscard]] internal::LogicObjectImpl& impl();

        /**
         * Get the internal data for implementation specifics of #LogicObject.
         */
        [[nodiscard]] const internal::LogicObjectImpl& impl() const;

    protected:
        /**
        * Constructor of #LogicObject. User is not supposed to call this - LogcNodes are created by subclasses
        *
        * @param impl implementation details of the #LogicObject
        */
        explicit LogicObject(std::unique_ptr<internal::LogicObjectImpl> impl) noexcept;

        /**
        * Stores internal data for implementation specifics of #LogicObject.
        */
        internal::LogicObjectImpl& m_impl;

        friend class internal::ApiObjects;
    };
}
