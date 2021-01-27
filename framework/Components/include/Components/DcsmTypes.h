//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENT_DCSMTYPES_H
#define RAMSES_COMPONENT_DCSMTYPES_H

#include "Common/StronglyTypedValue.h"
#include "Utils/LoggingUtils.h"
#include "Utils/StringOutputSpecialWrapper.h"

namespace ramses_internal
{
    struct ContentIDTag;
    using ContentID = StronglyTypedValue<uint64_t, 0, ContentIDTag>;

    struct CategoryTag;
    using Category = StronglyTypedValue<uint64_t, 0, CategoryTag>;

    struct ProviderIDTag;
    using ProviderID = StronglyTypedValue<uint64_t, 0, ProviderIDTag>;

    enum class ETechnicalContentType : uint32_t
    {
        RamsesSceneID,
        WaylandIviSurfaceID,
        Invalid
    };

    struct TechnicalContentDescriptorTag;
    using TechnicalContentDescriptor = StronglyTypedValue<uint64_t, 0, TechnicalContentDescriptorTag>;

    enum class EDcsmState : uint64_t
    {
        Offered,
        Assigned,
        Ready,
        Shown,
        AcceptStopOffer,
    };

    struct AnimationInformation
    {
        uint64_t startTimeStamp;
        uint64_t finishedTimeStamp;

        bool operator==(const AnimationInformation& rhs) const
        {
            return startTimeStamp == rhs.startTimeStamp && finishedTimeStamp == rhs.finishedTimeStamp;
        }
    };


    // enum names
    static const char* TechnicalContentTypeNames[] =
    {
        "RamsesSceneID",
        "WaylandIviSurfaceID",
        "Invalid",
    };

    static const char* DcsmStateNames[] =
    {
        "Offered",
        "Assigned",
        "Ready",
        "Shown",
        "AcceptStopOffer",
    };
}

MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::ContentID, ramses::ContentID)
MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::Category, ramses::Category)

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::ProviderID)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::TechnicalContentDescriptor)

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::ETechnicalContentType,
                                        "ETechnicalContentType",
                                        ramses_internal::TechnicalContentTypeNames,
                                        ramses_internal::ETechnicalContentType::Invalid);
MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::EDcsmState,
                                        "EDcsmState",
                                        ramses_internal::DcsmStateNames,
                                        ramses_internal::EDcsmState::AcceptStopOffer);

#endif
