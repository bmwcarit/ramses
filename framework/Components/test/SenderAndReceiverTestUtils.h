//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SENDERANDRECEIVERTESTUTILS_H
#define RAMSES_SENDERANDRECEIVERTESTUTILS_H

#include "gmock/gmock.h"


ACTION_TEMPLATE(AppendArgToVector,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(pointer))
{
    UNUSED(arg9);
    UNUSED(arg8);
    UNUSED(arg7);
    UNUSED(arg6);
    UNUSED(arg5);
    UNUSED(arg4);
    UNUSED(arg3);
    UNUSED(arg2);
    UNUSED(arg1);
    UNUSED(arg0);
    pointer->push_back(std::move(std::get<k>(args)));
}

ACTION_TEMPLATE(ExtendArgToVector,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(pointer))
{
    UNUSED(arg9);
    UNUSED(arg8);
    UNUSED(arg7);
    UNUSED(arg6);
    UNUSED(arg5);
    UNUSED(arg4);
    UNUSED(arg3);
    UNUSED(arg2);
    UNUSED(arg1);
    UNUSED(arg0);
    pointer->insert(pointer->end(), std::get<k>(args).begin(), std::get<k>(args).end());
}

#endif
