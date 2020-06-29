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
#include "CategoryInfoUpdateImpl.h"

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

    inline void PrintTo(const CategoryInfo& categoryInfo, ::std::ostream* os)
    {
        *os << "ri:CategoryInfo" << StringOutputStream::ToString(categoryInfo).stdRef();
    }

    inline void PrintTo(const AnimationInformation& ai, ::std::ostream* os)
    {
        *os << "ri:AnimationInformation[" << ai.startTimeStamp << ";" << ai.finishedTimeStamp << "]";
    }

    inline void PrintTo(const EDcsmState& state, ::std::ostream* os)
    {
        *os << "ri:" << EnumToString(state);
    }

    inline void PrintTo(const DcsmMetadata& metadata, ::std::ostream* os)
    {
        *os << "ri:DcsmMetadata" << StringOutputStream::ToString(metadata).stdRef();
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

    inline void PrintTo(const ramses::CategoryInfoUpdate& categoryInfo, ::std::ostream* os)
    {
        *os << "r:CategoryInfoUpdate" << ramses_internal::StringOutputStream::ToString(categoryInfo.impl.getCategoryInfo()).stdRef();
    }
}

#endif
