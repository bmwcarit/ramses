//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDARGUMENTSUTILS_H
#define RAMSES_RAMSHCOMMANDARGUMENTSUTILS_H

// helper macros for class generation
#define RAMSH_REPEAT_0(WHAT)
#define RAMSH_REPEAT_1(WHAT) WHAT(1)
#define RAMSH_REPEAT_2(WHAT) RAMSH_REPEAT_1(WHAT)WHAT(2)
#define RAMSH_REPEAT_3(WHAT) RAMSH_REPEAT_2(WHAT)WHAT(3)
#define RAMSH_REPEAT_4(WHAT) RAMSH_REPEAT_3(WHAT)WHAT(4)

#define RAMSH_REPEAT(N,WHAT) RAMSH_REPEAT_##N(WHAT)

#define RAMSH_REPEAT2_0(WHAT1,WHAT2) RAMSH_REPEAT_4(WHAT2)
#define RAMSH_REPEAT2_1(WHAT1,WHAT2) RAMSH_REPEAT_1(WHAT1)RAMSH_REPEAT_3(WHAT2)
#define RAMSH_REPEAT2_2(WHAT1,WHAT2) RAMSH_REPEAT_2(WHAT1)RAMSH_REPEAT_2(WHAT2)
#define RAMSH_REPEAT2_3(WHAT1,WHAT2) RAMSH_REPEAT_3(WHAT1)RAMSH_REPEAT_1(WHAT2)
#define RAMSH_REPEAT2_4(WHAT1,WHAT2) RAMSH_REPEAT_4(WHAT1)

#define RAMSH_REPEAT2(N,WHAT1,WHAT2) RAMSH_REPEAT2_##N(WHAT1,WHAT2)

#define RAMSH_COND_CALL2(CONDITION,WHAT) WHAT##_##CONDITION
#define RAMSH_COND_CALL(CONDITION,WHAT) RAMSH_COND_CALL2(CONDITION,WHAT)

namespace ramses_internal
{
    namespace ramsh_utils
    {
        // returns type N of a list of types
        template<Int32 N, typename T1, typename T2 = void, typename T3 = void, typename T4 = void, typename T5 = void>
        struct SelectType;

        template<typename T1, typename T2, typename T3, typename T4, typename T5>
        struct SelectType<0, T1, T2, T3, T4, T5>
        {
            typedef T1 type;
        };

        template<Int32 N, typename T1, typename T2, typename T3, typename T4, typename T5>
        struct SelectType
        {
            typedef typename SelectType<N - 1, T2, T3, T4, T5>::type type;
        };
    }
}

#endif

