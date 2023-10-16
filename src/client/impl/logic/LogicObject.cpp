//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicObject.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
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
