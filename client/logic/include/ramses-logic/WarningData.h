//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>

namespace rlogic
{
    class LogicObject;

    /**
    * #EWarningType lists the types of content warnings issued by #rlogic::LogicEngine::validate
    */
    enum class EWarningType : int
    {
        Performance,        ///< Warns about possibly optimize-able performance overhead
        UnsafeDataState,    ///< Warns about possible data races, potential data loss or otherwise unsafe data
        UninitializedData,  ///< Warns about uninitialized data which may result in unexpected behavior
        PrecisionLoss,      ///< Warns about possible precision issues, e.g. casting large types to smaller types
        UnusedContent,      ///< Warns about unused content which might be removed altogether
        DuplicateContent,   ///< Warns about duplicate content which might be possible to merge/optimize
        Other,              ///< Warning does not match any of the existing categories (this is used for new warnings for API compatibility)
    };

    /**
     * Holds information about a warning returned by #rlogic::LogicEngine::validate()
     */
    struct WarningData
    {
        /**
         * Error description as human-readable text.
         */
        std::string message;

        /**
         * Semantic type of the warning.
         */
        EWarningType type;

        /**
         * The #rlogic::LogicObject which caused the warning. Can be nullptr if the warning was not originating from a specific object.
         */
        const LogicObject* object;
    };

    /**
     * Returns the string representation of a given warning type. Use this to display a human-readible text
     * to content creators.
     */
    constexpr const char* GetVerboseDescription(EWarningType warningType)
    {
        switch (warningType)
        {
        case EWarningType::Performance:
            return "Performance";
        case EWarningType::UnsafeDataState:
            return "Unsafe Data State";
        case EWarningType::UninitializedData:
            return "Uninitialized Data";
        case EWarningType::PrecisionLoss:
            return "Precision Loss";
        case EWarningType::UnusedContent:
            return "Unused Content";
        case EWarningType::DuplicateContent:
            return "Duplicate Content";
        case EWarningType::Other:
            return "Other";
        }
        return "";
    }

}
