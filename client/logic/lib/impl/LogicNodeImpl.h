//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#pragma once

#include "impl/LogicObjectImpl.h"
#include "ramses-logic/EFeatureLevel.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace rlogic
{
    class Property;
}

namespace rlogic::internal
{
    struct LogicNodeRuntimeError { std::string message; };

    class LogicNodeImpl : public LogicObjectImpl
    {
    public:
        explicit LogicNodeImpl(std::string_view name, uint64_t id) noexcept;
        LogicNodeImpl(const LogicNodeImpl& other) = delete;
        LogicNodeImpl& operator=(const LogicNodeImpl& other) = delete;
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
        void setRootProperties(std::unique_ptr<Property> rootInput, std::unique_ptr<Property> rootOutput);

    private:
        std::unique_ptr<Property> m_inputs;
        std::unique_ptr<Property> m_outputs;
        // Dirty after creation (every node gets executed at least once after creation)
        bool                      m_dirty = true;
    };
}
