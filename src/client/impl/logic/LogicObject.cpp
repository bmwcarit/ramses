//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicObject.h"
#include "impl/logic/LogicObjectImpl.h"

namespace ramses
{
    LogicObject::LogicObject(std::unique_ptr<internal::LogicObjectImpl> impl) noexcept
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::LogicObjectImpl&>(SceneObject::m_impl) }
    {
    }

    internal::LogicObjectImpl& LogicObject::impl()
    {
        return m_impl;
    }

    const internal::LogicObjectImpl& LogicObject::impl() const
    {
        return m_impl;
    }
}
