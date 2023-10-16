//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#pragma once

#include "impl/logic/LogicObjectImpl.h"
#include "impl/logic/PropertyImpl.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace ramses::internal
{
    struct LogicNodeRuntimeError { std::string message; };

    class LogicNodeImpl : public LogicObjectImpl
    {
    public:
        explicit LogicNodeImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id) noexcept;
        ~LogicNodeImpl() noexcept override;

        [[nodiscard]] Property*       getInputs();
        [[nodiscard]] const Property* getInputs() const;

        // Virtual because of LuaInterface
        [[nodiscard]] virtual Property* getOutputs();
        [[nodiscard]] virtual const Property* getOutputs() const;

        virtual void createRootProperties() = 0;
        virtual std::optional<LogicNodeRuntimeError> update() = 0;

        void setDirty(bool dirty);
        [[nodiscard]] bool isDirty() const;

    protected:
        void setRootProperties(std::unique_ptr<PropertyImpl> rootInput, std::unique_ptr<PropertyImpl> rootOutput);

    private:
        PropertyUniquePtr m_inputs;
        PropertyUniquePtr m_outputs;

        // Dirty after creation (every node gets executed at least once after creation)
        bool m_dirty = true;
    };
}
