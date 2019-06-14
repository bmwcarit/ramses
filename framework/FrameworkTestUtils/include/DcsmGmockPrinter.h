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
        *os << "ContentID:" << id.getValue();
    }

    inline void PrintTo(const Category& id, ::std::ostream* os)
    {
        *os << "Category:" << id.getValue();
    }

    inline void PrintTo(const TechnicalContentDescriptor& id, ::std::ostream* os)
    {
        *os << "TechnicalContentDescriptor:" << id.getValue();
    }

    inline void PrintTo(const SizeInfo& si, ::std::ostream* os)
    {
        *os << "SizeInfo[" << si.width << "x" << si.height << "]";
    }

    inline void PrintTo(const AnimationInformation& ai, ::std::ostream* os)
    {
        *os << "AnimationInformation[" << ai.startTimeStamp << ";" << ai.finishedTimeStamp << "]";
    }

    inline void PrintTo(const EDcsmStatus& status, ::std::ostream* os)
    {
        *os << EnumToString(status);
    }
}

namespace ramses
{
    inline void PrintTo(const ContentID& id, ::std::ostream* os)
    {
        *os << "ContentID:" << id.getValue();
    }

    inline void PrintTo(const Category& id, ::std::ostream* os)
    {
        *os << "Category:" << id.getValue();
    }

    inline void PrintTo(const TechnicalContentDescriptor& id, ::std::ostream* os)
    {
        *os << "TechnicalContentDescriptor:" << id.getValue();
    }

    inline void PrintTo(const SizeInfo& si, ::std::ostream* os)
    {
        *os << "SizeInfo[" << si.width << "x" << si.height << "]";
    }

    inline void PrintTo(const AnimationInformation& ai, ::std::ostream* os)
    {
        *os << "AnimationInformation[" << ai.startTime << ";" << ai.finishTime << "]";
    }

    inline void PrintTo(const EDcsmStatus& status, ::std::ostream* os)
    {
        *os << EnumToString(static_cast<ramses_internal::EDcsmStatus>(status));
    }
}

#endif
