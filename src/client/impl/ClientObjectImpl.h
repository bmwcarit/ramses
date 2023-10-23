//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesObjectImpl.h"

#include <string_view>

namespace ramses::internal
{
    class RamsesClientImpl;
    class ErrorReporting;

    class ClientObjectImpl : public RamsesObjectImpl
    {
    public:
        explicit ClientObjectImpl(RamsesClientImpl& client, ERamsesObjectType type, std::string_view name);
        ~ClientObjectImpl() override;

        [[nodiscard]] const RamsesClientImpl& getClientImpl() const;
        [[nodiscard]] RamsesClientImpl&       getClientImpl();
        [[nodiscard]] ErrorReporting&         getErrorReporting() const; // const so error can be set from const methods of derived classes

        [[nodiscard]] bool isFromTheSameClientAs(const ClientObjectImpl& otherObject) const;

    private:
        RamsesClientImpl& m_client;
    };
}
