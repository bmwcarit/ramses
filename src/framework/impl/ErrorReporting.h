//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/Issue.h"
#include "ramses/framework/APIExport.h"
#include <optional>
#include <mutex>

namespace ramses
{
    class RamsesObject;
}

namespace ramses::internal
{
    class RamsesObjectImpl;

    class RAMSES_IMPL_EXPORT ErrorReporting
    {
    public:
        void reset();
        void set(std::string errorMessage, const RamsesObject* object = nullptr);
        void set(std::string errorMessage, const RamsesObjectImpl& object);

        [[nodiscard]] std::optional<ramses::Issue> getError() const;

    private:
        std::optional<ramses::Issue> m_error;
        mutable std::recursive_mutex m_lock;
    };
}
