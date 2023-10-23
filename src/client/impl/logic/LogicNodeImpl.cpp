//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/LogicNodeImpl.h"

#include "ramses/client/logic/Property.h"

#include "impl/logic/PropertyImpl.h"

namespace ramses::internal
{
    LogicNodeImpl::LogicNodeImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id) noexcept
        : LogicObjectImpl(scene, name, id)
    {
    }

    LogicNodeImpl::~LogicNodeImpl() noexcept = default;

    Property* LogicNodeImpl::getInputs()
    {
        return m_inputs.get();
    }

    const Property* LogicNodeImpl::getInputs() const
    {
        return m_inputs.get();
    }

    const Property* LogicNodeImpl::getOutputs() const
    {
        return m_outputs.get();
    }

    Property* LogicNodeImpl::getOutputs()
    {
        return m_outputs.get();
    }

    void LogicNodeImpl::setDirty(bool dirty)
    {
        m_dirty = dirty;
    }

    bool LogicNodeImpl::isDirty() const
    {
        return m_dirty;
    }

    void LogicNodeImpl::setRootProperties(std::unique_ptr<PropertyImpl> rootInput, std::unique_ptr<PropertyImpl> rootOutput)
    {
        assert(!m_inputs);
        assert(!m_outputs);

        if (rootInput)
        {
            m_inputs = PropertyImpl::CreateProperty(std::move(rootInput));
            m_inputs->impl().setLogicNode(*this);
        }

        if (rootOutput)
        {
            m_outputs = PropertyImpl::CreateProperty(std::move(rootOutput));
            m_outputs->impl().setLogicNode(*this);
        }
    }
}
