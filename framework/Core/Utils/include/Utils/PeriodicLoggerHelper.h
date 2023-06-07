//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERIODICLOGGERHELPER_H
#define RAMSES_PERIODICLOGGERHELPER_H

#include "Collections/StringOutputStream.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/StatisticCollection.h"

namespace ramses_internal
{
    template <typename DataType>
    void logStatisticSummaryEntry(StringOutputStream& stream, const SummaryEntry<DataType>& summary, uint32_t numberTimeIntervals)
    {
        if (numberTimeIntervals > 0)
        {
            if (summary.minValue == summary.maxValue)
                stream << "(" << summary.minValue << ")";
            else
                stream << "(" << summary.minValue << "/" << summary.maxValue << "/" << summary.sum / numberTimeIntervals << ")";
        }
        else
        {
            stream << "(n.a.)";
        }
    }

    template <typename DataType, size_t N>
    void logStatisticSummaryEntry(StringOutputStream& stream, const FirstNElementsSummaryEntry<DataType, N>& summary, uint32_t numberTimeIntervals)
    {
        if (numberTimeIntervals > 0 && summary.array.size() > 0)
        {
            stream << "(";
            auto separator{""};
            for (const auto& el : summary.array)
            {
                stream << separator << el;
                separator = "/";
            }
            if (numberTimeIntervals > summary.array.size())
            {
                stream << "/...)";
            }
            else
            {
                stream << ")";
            }
        }
        else
        {
            stream << "(n.a.)";
        }
    }
}

#endif
