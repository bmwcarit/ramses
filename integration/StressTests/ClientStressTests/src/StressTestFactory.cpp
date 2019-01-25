//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StressTestFactory.h"
#include "ResourceStressTests.h"
#include "Utils/LoggingUtils.h"

using namespace ramses_internal;

//NOTE: PLEASE UPDATE NUMBERS OF TESTS RUN IN SMOKE TESTS IF ANY OF THE VALUES IN THIS ENUM IS CHANGED
enum ETest
{
    //always begin with 0, do not break continuity
    ETest_createDestroyVector4fArray = 0,
    ETest_saveVector4fArray,
    ETest_loadVector4fArray,
    ETest_loadVector4fArrayAsync,
    ETest_saveLoadVector4fArray,
    ETest_saveLoadVector4fArrayAsync,
    ETest_createDestroyTexture2D,
    ETest_saveTexture2D,
    ETest_loadTexture2D,
    ETest_loadTexture2DAsync,
    ETest_saveLoadTexture2D,
    ETest_saveLoadTexture2DAsync,
    ETest_createDestroyEffect,
    ETest_saveEffect,
    ETest_loadEffect,
    ETest_loadEffectAsync,
    ETest_saveLoadEffect,
    ETest_saveLoadEffectAsync,

    //keep this at the end
    ETest_NUMBER_OF_TESTS
};

static_assert(18 == ETest_NUMBER_OF_TESTS, "Update test numbers in smoke tests as well");

const char* StressTestNames[] =
{
    "ETest_createDestroyVector4fArray",
    "ETest_saveVector4fArray",
    "ETest_loadVector4fArray",
    "ETest_loadVector4fArrayAsync",
    "ETest_saveLoadVector4fArray",
    "ETest_saveLoadVector4fArrayAsync",
    "ETest_createDestroyTexture2D",
    "ETest_saveTexture2D",
    "ETest_loadTexture2D",
    "ETest_loadTexture2DAsync",
    "ETest_saveLoadTexture2D",
    "ETest_saveLoadTexture2DAsync",
    "ETest_createDestroyEffect",
    "ETest_saveEffect",
    "ETest_loadEffect",
    "ETest_loadEffectAsync",
    "ETest_saveLoadEffect",
    "ETest_saveLoadEffectAsync",
};

ENUM_TO_STRING(ETest, StressTestNames, ETest_NUMBER_OF_TESTS);


StressTestPtr StressTestFactory::CreateTest(uint32_t testIndex, int32_t argc, const char* argv[])
{
    const ETest testToCreate = static_cast<ETest>(testIndex);

    switch (testToCreate)
    {
    case ETest_createDestroyVector4fArray:
        return StressTestPtr(new CreateDestroyVector4fArray(argc, argv));
    case ETest_saveVector4fArray:
        return StressTestPtr(new SaveVector4fArray(argc, argv));
    case ETest_loadVector4fArray:
        return StressTestPtr(new LoadVector4fArray(argc, argv));
    case ETest_loadVector4fArrayAsync:
        return StressTestPtr(new LoadVector4fArrayAsync(argc, argv));
    case ETest_saveLoadVector4fArray:
        return StressTestPtr(new SaveLoadVector4fArray(argc, argv));
    case ETest_saveLoadVector4fArrayAsync:
        return StressTestPtr(new SaveLoadVector4fArrayAsync(argc, argv));
    case ETest_createDestroyTexture2D:
        return StressTestPtr(new CreateDestroyTexture2D(argc, argv));
    case ETest_saveTexture2D:
        return StressTestPtr(new SaveTexture2D(argc, argv));
    case ETest_loadTexture2D:
        return StressTestPtr(new LoadTexture2D(argc, argv));
    case ETest_loadTexture2DAsync:
        return StressTestPtr(new LoadTexture2DAsync(argc, argv));
    case ETest_saveLoadTexture2D:
        return StressTestPtr(new SaveLoadTexture2D(argc, argv));
    case ETest_saveLoadTexture2DAsync:
        return StressTestPtr(new SaveLoadTexture2DAsync(argc, argv));
    case ETest_createDestroyEffect:
        return StressTestPtr(new CreateDestroyEffect(argc, argv));
    case ETest_saveEffect:
        return StressTestPtr(new SaveEffect(argc, argv));
    case ETest_loadEffect:
        return StressTestPtr(new LoadEffect(argc, argv));
    case ETest_loadEffectAsync:
        return StressTestPtr(new LoadEffectAsync(argc, argv));
    case ETest_saveLoadEffect:
        return StressTestPtr(new SaveLoadEffect(argc, argv));
    case ETest_saveLoadEffectAsync:
        return StressTestPtr(new SaveLoadEffectAsync(argc, argv));
    default:
        assert(false);
        return nullptr;
    }
}

uint32_t StressTestFactory::GetNumberOfTests()
{
    return static_cast<uint32_t>(ETest_NUMBER_OF_TESTS);
}

const char* StressTestFactory::GetNameOfTest(uint32_t testIndex)
{
    return EnumToString(static_cast<ETest>(testIndex));
}
