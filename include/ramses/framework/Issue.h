//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>

namespace ramses
{
    class RamsesObject;

    enum class EIssueType
    {
        Error,
        Warning,
    };

    /**
     * Holds information about a message returned by #ramses::Scene::validate()
     */
    struct Issue
    {
        /**
         * Issue classification
         */
        EIssueType type = EIssueType::Warning;

        /**
         * human-readable text message.
         */
        std::string message;

        /**
         * The #ramses::RamsesObject which raised the message. Can be nullptr if the warning was not originating from a specific object.
         */
        const RamsesObject* object = nullptr;

        bool operator==(const Issue& rhs) const
        {
            return type == rhs.type
                && object == rhs.object
                && message == rhs.message;
        }

        bool operator!=(const Issue& rhs) const
        {
            return !operator==(rhs);
        }
    };
}
