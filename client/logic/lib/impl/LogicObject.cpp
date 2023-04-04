//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicObject.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/AnchorPoint.h"
#include "impl/LogicObjectImpl.h"

namespace rlogic
{
    LogicObject::LogicObject(std::unique_ptr<internal::LogicObjectImpl> impl) noexcept
        : m_impl{ std::move(impl) }
    {
    }

    LogicObject::~LogicObject() noexcept = default;

    std::string_view LogicObject::getName() const
    {
        return m_impl->getName();
    }

    uint64_t LogicObject::getId() const
    {
        return m_impl->getId();
    }

    bool LogicObject::setUserId(uint64_t highId, uint64_t lowId)
    {
        return m_impl->setUserId(highId, lowId);
    }

    std::pair<uint64_t, uint64_t> LogicObject::getUserId() const
    {
        return m_impl->getUserId();
    }

    template <typename T>
    const T* LogicObject::internalCast() const
    {
        return dynamic_cast<const T*>(this);
    }

    template <typename T>
    T* LogicObject::internalCast()
    {
        return dynamic_cast<T*>(this);
    }

    bool LogicObject::setName(std::string_view name)
    {
        return m_impl->setName(name);
    }

    template RAMSES_API const LogicObject*              LogicObject::internalCast() const;
    template RAMSES_API const LogicNode*                LogicObject::internalCast() const;
    template RAMSES_API const RamsesBinding*            LogicObject::internalCast() const;
    template RAMSES_API const LuaModule*                LogicObject::internalCast() const;
    template RAMSES_API const LuaScript*                LogicObject::internalCast() const;
    template RAMSES_API const LuaInterface*             LogicObject::internalCast() const;
    template RAMSES_API const RamsesNodeBinding*        LogicObject::internalCast() const;
    template RAMSES_API const RamsesAppearanceBinding*  LogicObject::internalCast() const;
    template RAMSES_API const RamsesCameraBinding*      LogicObject::internalCast() const;
    template RAMSES_API const RamsesRenderPassBinding*  LogicObject::internalCast() const;
    template RAMSES_API const RamsesRenderGroupBinding* LogicObject::internalCast() const;
    template RAMSES_API const RamsesMeshNodeBinding*    LogicObject::internalCast() const;
    template RAMSES_API const SkinBinding*              LogicObject::internalCast() const;
    template RAMSES_API const DataArray*                LogicObject::internalCast() const;
    template RAMSES_API const AnimationNode*            LogicObject::internalCast() const;
    template RAMSES_API const TimerNode*                LogicObject::internalCast() const;
    template RAMSES_API const AnchorPoint*              LogicObject::internalCast() const;

    template RAMSES_API LogicObject*              LogicObject::internalCast();
    template RAMSES_API LogicNode*                LogicObject::internalCast();
    template RAMSES_API RamsesBinding*            LogicObject::internalCast();
    template RAMSES_API LuaModule*                LogicObject::internalCast();
    template RAMSES_API LuaScript*                LogicObject::internalCast();
    template RAMSES_API LuaInterface*             LogicObject::internalCast();
    template RAMSES_API RamsesNodeBinding*        LogicObject::internalCast();
    template RAMSES_API RamsesAppearanceBinding*  LogicObject::internalCast();
    template RAMSES_API RamsesCameraBinding*      LogicObject::internalCast();
    template RAMSES_API RamsesRenderPassBinding*  LogicObject::internalCast();
    template RAMSES_API RamsesRenderGroupBinding* LogicObject::internalCast();
    template RAMSES_API RamsesMeshNodeBinding*    LogicObject::internalCast();
    template RAMSES_API SkinBinding*              LogicObject::internalCast();
    template RAMSES_API DataArray*                LogicObject::internalCast();
    template RAMSES_API AnimationNode*            LogicObject::internalCast();
    template RAMSES_API TimerNode*                LogicObject::internalCast();
    template RAMSES_API AnchorPoint*              LogicObject::internalCast();
}
