//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IPERIODICLOGSUPPLIER_H
#define RAMSES_IPERIODICLOGSUPPLIER_H

namespace ramses_internal
{
    class IPeriodicLogSupplier
    {
    public:
        virtual ~IPeriodicLogSupplier() {};
        virtual void triggerLogMessageForPeriodicLog() const = 0;
    };
}

#endif

