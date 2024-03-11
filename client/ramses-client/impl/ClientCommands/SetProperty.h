//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "Ramsh/RamshCommandArguments.h"
#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/RamsesObjectTypes.h"

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class SetProperty : public RamshCommand
    {
    public:
        enum class Type
        {
            Visible,
            Uniform,
            DepthWrite,
            DepthFunc,
            Invalid,
        };

        static Type GetPropertyType(ramses::ERamsesObjectType objType, const std::string& name);

        explicit SetProperty(ramses::RamsesClientImpl& client);
        virtual bool executeInput(const std::vector<std::string>& input) override;
    private:
        ramses::RamsesClientImpl& m_client;
    };

    class SetPropertyAll : public RamshCommand
    {
    public:
        explicit SetPropertyAll(ramses::RamsesClientImpl& client);
        virtual bool executeInput(const std::vector<std::string>& input) override;
    private:
        ramses::RamsesClientImpl& m_client;
    };
}

