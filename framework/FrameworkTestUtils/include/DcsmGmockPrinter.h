//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMGMOCKPRINTER_H
#define RAMSES_DCSMGMOCKPRINTER_H

#include "ramses-framework-api/DcsmApiTypes.h"
#include "Components/DcsmComponent.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    inline void PrintTo(const ContentID& id, ::std::ostream* os)
    {
        *os << "ri:ContentID:" << id.getValue();
    }

    inline void PrintTo(const Category& id, ::std::ostream* os)
    {
        *os << "ri:Category:" << id.getValue();
    }

    inline void PrintTo(const TechnicalContentDescriptor& id, ::std::ostream* os)
    {
        *os << "ri:TechnicalContentDescriptor:" << id.getValue();
    }

    inline void PrintTo(const SizeInfo& si, ::std::ostream* os)
    {
        *os << "ri:SizeInfo[" << si.width << "x" << si.height << "]";
    }

    inline void PrintTo(const AnimationInformation& ai, ::std::ostream* os)
    {
        *os << "ri:AnimationInformation[" << ai.startTimeStamp << ";" << ai.finishedTimeStamp << "]";
    }

    inline void PrintTo(const EDcsmState& state, ::std::ostream* os)
    {
        *os << "ri:" << EnumToString(state);
    }
}

namespace ramses
{
    inline void PrintTo(const ramses::ContentID& id, ::std::ostream* os)
    {
        *os << "r:ContentID:" << id.getValue();
    }

    inline void PrintTo(const ramses::Category& id, ::std::ostream* os)
    {
        *os << "r:Category:" << id.getValue();
    }

    inline void PrintTo(const ramses::TechnicalContentDescriptor& id, ::std::ostream* os)
    {
        *os << "r:TechnicalContentDescriptor:" << id.getValue();
    }

    inline void PrintTo(const ramses::SizeInfo& si, ::std::ostream* os)
    {
        *os << "r:SizeInfo[" << si.width << "x" << si.height << "]";
    }

    inline void PrintTo(const ramses::AnimationInformation& ai, ::std::ostream* os)
    {
        *os << "r:AnimationInformation[" << ai.startTime << ";" << ai.finishTime << "]";
    }
}

#endif
