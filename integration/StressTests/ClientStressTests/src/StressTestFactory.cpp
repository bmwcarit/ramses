//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StressTestFactory.h"
#include "TextStressTests.h"
#include "ResourceStressTests.h"
#include "Utils/LoggingUtils.h"

using namespace ramses_internal;

//NOTE: PLEASE UPDATE NUMBERS OF TESTS RUN IN SMOKE TESTS IF ANY OF THE VALUES IN THIS ENUM IS CHANGED
enum ETest
{
    //always begin with 0, do not break continuity
    ETest_createDestroyTextNodes = 0,
    ETest_addRemoveTextNodes,
    ETest_createDestroyFloatArray,
    ETest_saveFloatArray,
    ETest_loadFloatArray,
    ETest_loadFloatArrayAsync,
    ETest_saveLoadFloatArray,
    ETest_saveLoadFloatArrayAsync,
    ETest_createDestroyVector2fArray,
    ETest_saveVector2fArray,
    ETest_loadVector2fArray,
    ETest_loadVector2fArrayAsync,
    ETest_saveLoadVector2fArray,
    ETest_saveLoadVector2fArrayAsync,
    ETest_createDestroyVector3fArray,
    ETest_saveVector3fArray,
    ETest_loadVector3fArray,
    ETest_loadVector3fArrayAsync,
    ETest_saveLoadVector3fArray,
    ETest_saveLoadVector3fArrayAsync,
    ETest_createDestroyVector4fArray,
    ETest_saveVector4fArray,
    ETest_loadVector4fArray,
    ETest_loadVector4fArrayAsync,
    ETest_saveLoadVector4fArray,
    ETest_saveLoadVector4fArrayAsync,
    ETest_createDestroyUInt16Array,
    ETest_saveUInt16Array,
    ETest_loadUInt16Array,
    ETest_loadUInt16ArrayAsync,
    ETest_saveLoadUInt16Array,
    ETest_saveLoadUInt16ArrayAsync,
    ETest_createDestroyUInt32Array,
    ETest_saveUInt32Array,
    ETest_loadUInt32Array,
    ETest_loadUInt32ArrayAsync,
    ETest_saveLoadUInt32Array,
    ETest_saveLoadUInt32ArrayAsync,
    ETest_createDestroyTexture2D,
    ETest_saveTexture2D,
    ETest_loadTexture2D,
    ETest_loadTexture2DAsync,
    ETest_saveLoadTexture2D,
    ETest_saveLoadTexture2DAsync,
    ETest_createDestroyTexture3D,
    ETest_saveTexture3D,
    ETest_loadTexture3D,
    ETest_loadTexture3DAsync,
    ETest_saveLoadTexture3D,
    ETest_saveLoadTexture3DAsync,
    ETest_createDestroyTextureCube,
    ETest_saveTextureCube,
    ETest_loadTextureCube,
    ETest_loadTextureCubeAsync,
    ETest_saveLoadTextureCube,
    ETest_saveLoadTextureCubeAsync,
    ETest_createDestroyEffect,
    ETest_saveEffect,
    ETest_loadEffect,
    ETest_loadEffectAsync,
    ETest_saveLoadEffect,
    ETest_saveLoadEffectAsync,

    //keep this at the end
    ETest_NUMBER_OF_TESTS
};

static_assert(62 == ETest_NUMBER_OF_TESTS, "Update test numbers in smoke tests as well");

const char* StressTestNames[] =
{
    "ETest_createDestroyTextNodes",
    "ETest_addRemoveTextNodes",
    "ETest_createDestroyFloatArray",
    "ETest_saveFloatArray",
    "ETest_loadFloatArray",
    "ETest_loadFloatArrayAsync",
    "ETest_saveLoadFloatArray",
    "ETest_saveLoadFloatArrayAsync",
    "ETest_createDestroyVector2fArray",
    "ETest_saveVector2fArray",
    "ETest_loadVector2fArray",
    "ETest_loadVector2fArrayAsync",
    "ETest_saveLoadVector2fArray",
    "ETest_saveLoadVector2fArrayAsync",
    "ETest_createDestroyVector3fArray",
    "ETest_saveVector3fArray",
    "ETest_loadVector3fArray",
    "ETest_loadVector3fArrayAsync",
    "ETest_saveLoadVector3fArray",
    "ETest_saveLoadVector3fArrayAsync",
    "ETest_createDestroyVector4fArray",
    "ETest_saveVector4fArray",
    "ETest_loadVector4fArray",
    "ETest_loadVector4fArrayAsync",
    "ETest_saveLoadVector4fArray",
    "ETest_saveLoadVector4fArrayAsync",
    "ETest_createDestroyUInt16Array",
    "ETest_saveUInt16Array",
    "ETest_loadUInt16Array",
    "ETest_loadUInt16ArrayAsync",
    "ETest_saveLoadUInt16Array",
    "ETest_saveLoadUInt16ArrayAsync",
    "ETest_createDestroyUInt32Array",
    "ETest_saveUInt32Array",
    "ETest_loadUInt32Array",
    "ETest_loadUInt32ArrayAsync",
    "ETest_saveLoadUInt32Array",
    "ETest_saveLoadUInt32ArrayAsync",
    "ETest_createDestroyTexture2D",
    "ETest_saveTexture2D",
    "ETest_loadTexture2D",
    "ETest_loadTexture2DAsync",
    "ETest_saveLoadTexture2D",
    "ETest_saveLoadTexture2DAsync",
    "ETest_createDestroyTexture3D",
    "ETest_saveTexture3D",
    "ETest_loadTexture3D",
    "ETest_loadTexture3DAsync",
    "ETest_saveLoadTexture3D",
    "ETest_saveLoadTexture3DAsync",
    "ETest_createDestroyTextureCube",
    "ETest_saveTextureCube",
    "ETest_loadTextureCube",
    "ETest_loadTextureCubeAsync",
    "ETest_saveLoadTextureCube",
    "ETest_saveLoadTextureCubeAsync",
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
    case ETest_createDestroyTextNodes:
        return StressTestPtr(new CreateDestroyTextNodes(argc, argv));
    case ETest_addRemoveTextNodes:
        return StressTestPtr(new AddRemoveTextNodes(argc, argv));
    case ETest_createDestroyFloatArray:
        return StressTestPtr(new CreateDestroyFloatArray(argc, argv));
    case ETest_saveFloatArray:
        return StressTestPtr(new SaveFloatArray(argc, argv));
    case ETest_loadFloatArray:
        return StressTestPtr(new LoadFloatArray(argc, argv));
    case ETest_loadFloatArrayAsync:
        return StressTestPtr(new LoadFloatArrayAsync(argc, argv));
    case ETest_saveLoadFloatArray:
        return StressTestPtr(new SaveLoadFloatArray(argc, argv));
    case ETest_saveLoadFloatArrayAsync:
        return StressTestPtr(new SaveLoadFloatArrayAsync(argc, argv));
    case ETest_createDestroyVector2fArray:
        return StressTestPtr(new CreateDestroyVector2fArray(argc, argv));
    case ETest_saveVector2fArray:
        return StressTestPtr(new SaveVector2fArray(argc, argv));
    case ETest_loadVector2fArray:
        return StressTestPtr(new LoadVector2fArray(argc, argv));
    case ETest_loadVector2fArrayAsync:
        return StressTestPtr(new LoadVector2fArrayAsync(argc, argv));
    case ETest_saveLoadVector2fArray:
        return StressTestPtr(new SaveLoadVector2fArray(argc, argv));
    case ETest_saveLoadVector2fArrayAsync:
        return StressTestPtr(new SaveLoadVector2fArrayAsync(argc, argv));
    case ETest_createDestroyVector3fArray:
        return StressTestPtr(new CreateDestroyVector3fArray(argc, argv));
    case ETest_saveVector3fArray:
        return StressTestPtr(new SaveVector3fArray(argc, argv));
    case ETest_loadVector3fArray:
        return StressTestPtr(new LoadVector3fArray(argc, argv));
    case ETest_loadVector3fArrayAsync:
        return StressTestPtr(new LoadVector3fArrayAsync(argc, argv));
    case ETest_saveLoadVector3fArray:
        return StressTestPtr(new SaveLoadVector3fArray(argc, argv));
    case ETest_saveLoadVector3fArrayAsync:
        return StressTestPtr(new SaveLoadVector3fArrayAsync(argc, argv));
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
    case ETest_createDestroyUInt16Array:
        return StressTestPtr(new CreateDestroyUInt16Array(argc, argv));
    case ETest_saveUInt16Array:
        return StressTestPtr(new SaveUInt16Array(argc, argv));
    case ETest_loadUInt16Array:
        return StressTestPtr(new LoadUInt16Array(argc, argv));
    case ETest_loadUInt16ArrayAsync:
        return StressTestPtr(new LoadUInt16ArrayAsync(argc, argv));
    case ETest_saveLoadUInt16Array:
        return StressTestPtr(new SaveLoadUInt16Array(argc, argv));
    case ETest_saveLoadUInt16ArrayAsync:
        return StressTestPtr(new SaveLoadUInt16ArrayAsync(argc, argv));
    case ETest_createDestroyUInt32Array:
        return StressTestPtr(new CreateDestroyUInt32Array(argc, argv));
    case ETest_saveUInt32Array:
        return StressTestPtr(new SaveUInt32Array(argc, argv));
    case ETest_loadUInt32Array:
        return StressTestPtr(new LoadUInt32Array(argc, argv));
    case ETest_loadUInt32ArrayAsync:
        return StressTestPtr(new LoadUInt32ArrayAsync(argc, argv));
    case ETest_saveLoadUInt32Array:
        return StressTestPtr(new SaveLoadUInt32Array(argc, argv));
    case ETest_saveLoadUInt32ArrayAsync:
        return StressTestPtr(new SaveLoadUInt32ArrayAsync(argc, argv));
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
    case ETest_createDestroyTexture3D:
        return StressTestPtr(new CreateDestroyTexture3D(argc, argv));
    case ETest_saveTexture3D:
        return StressTestPtr(new SaveTexture3D(argc, argv));
    case ETest_loadTexture3D:
        return StressTestPtr(new LoadTexture3D(argc, argv));
    case ETest_loadTexture3DAsync:
        return StressTestPtr(new LoadTexture3DAsync(argc, argv));
    case ETest_saveLoadTexture3D:
        return StressTestPtr(new SaveLoadTexture3D(argc, argv));
    case ETest_saveLoadTexture3DAsync:
        return StressTestPtr(new SaveLoadTexture3DAsync(argc, argv));
    case ETest_createDestroyTextureCube:
        return StressTestPtr(new CreateDestroyTextureCube(argc, argv));
    case ETest_saveTextureCube:
        return StressTestPtr(new SaveTextureCube(argc, argv));
    case ETest_loadTextureCube:
        return StressTestPtr(new LoadTextureCube(argc, argv));
    case ETest_loadTextureCubeAsync:
        return StressTestPtr(new LoadTextureCubeAsync(argc, argv));
    case ETest_saveLoadTextureCube:
        return StressTestPtr(new SaveLoadTextureCube(argc, argv));
    case ETest_saveLoadTextureCubeAsync:
        return StressTestPtr(new SaveLoadTextureCubeAsync(argc, argv));
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
