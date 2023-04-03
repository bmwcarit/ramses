//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include <string>
#include <ostream>

namespace rlogic
{
    /**
     * @brief Result class as a return object for functions
     */
    class Result
    {
    public:
        Result() = default;

        explicit Result(std::string msg)
            : m_message(std::move(msg))
        {
        }

        [[nodiscard]] bool ok() const
        {
            return m_message.empty();
        }

        [[nodiscard]] const std::string& getMessage() const
        {
            return m_message;
        }

        [[nodiscard]] bool operator==(const Result& rhs) const
        {
            return rhs.m_message == m_message;
        }

        [[nodiscard]] bool operator!=(const Result& rhs) const
        {
            return !(rhs == *this);
        }

    private:
        std::string m_message;
    };

    inline std::ostream& operator<<(std::ostream& os, const Result& result)
    {
        os << result.getMessage();
        return os;
    }
}
