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

namespace ramses_internal
{
    using ContentID = StronglyTypedValue<uint64_t, 0, struct ContentIDTag>;
    using Category = StronglyTypedValue<uint64_t, 0, struct CategoryTag> ;
    using ProviderID = StronglyTypedValue<uint64_t, 0, struct ProviderIDTag> ;

    enum class ETechnicalContentType : uint32_t
    {
        RamsesSceneID,
    };

    inline const char* EnumToString(ETechnicalContentType val)
    {
        switch (val)
        {
        case ETechnicalContentType::RamsesSceneID: return "ETechnicalContentType::RamsesSceneID";
        }
        return "ETechnicalContentType::<UNKNOWN>";
    }

    using TechnicalContentDescriptor = StronglyTypedValue<uint64_t, 0, struct TechnicalContentDescriptorTag>;

    struct SizeInfo
    {
        uint32_t width;
        uint32_t height;

        bool operator==(const SizeInfo& rhs) const
        {
            return width == rhs.width && height == rhs.height;
        }

        bool operator!=(const SizeInfo& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class EDcsmState : uint32_t
    {
        Offered,
        Assigned,
        Ready,
        Shown,
        AcceptStopOffer,
    };

    inline const char* EnumToString(EDcsmState val)
    {
        switch (val)
        {
        case EDcsmState::Offered: return "EDcsmState::Offered";
        case EDcsmState::Assigned: return "EDcsmState::Assigned";
        case EDcsmState::Ready: return "EDcsmState::Ready";
        case EDcsmState::Shown: return "EDcsmState::Shown";
        case EDcsmState::AcceptStopOffer: return "EDcsmState::AcceptStopOffer";
        }
        return "EDcsmState::<UNKNOWN>";
    }

    struct AnimationInformation
    {
        uint64_t startTimeStamp;
        uint64_t finishedTimeStamp;

        bool operator==(const AnimationInformation& rhs) const
        {
            return startTimeStamp == rhs.startTimeStamp && finishedTimeStamp == rhs.finishedTimeStamp;
        }
    };
}

#endif
