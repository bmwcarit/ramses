//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SPLINESOLVERTEST_H
#define RAMSES_SPLINESOLVERTEST_H

#include "framework_common_gmock_header.h"
#include "AnimationTestUtils.h"
#include "Animation/SplineSolver.h"

namespace ramses_internal
{
    class ASpline : public testing::Test
    {
    public:
        ASpline()
        {
        }

    protected:
        void splineSolverOutOfRange(EInterpolationType interpolationType);
        void splineSolverValuesAtKeys(EInterpolationType interpolationType);
        void splineSolverInterpolateTypesTest(EInterpolationType interpolationType);
        template <typename EDataType>
        void splineSolverInterpolateTest(EInterpolationType interpolationType, const char* typeName);
        void splineSolverVisualTest(EInterpolationType interpolationType);
    };

    template void ASpline::splineSolverInterpolateTest<Bool>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Int32>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Int64>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<UInt32>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<UInt64>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Float>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Double>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Vector2>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Vector3>(EInterpolationType, const char*);
    template void ASpline::splineSolverInterpolateTest<Vector4>(EInterpolationType, const char*);
}

#endif
