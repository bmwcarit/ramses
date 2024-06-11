//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 320 es

precision highp float;

// non-UBO inputs/outputs
uniform highp mat4 mvpMatrix;
in vec3 a_position;
out vec4 v_fragColor;

layout(std140, binding=0) uniform UboTypeScalars
{
    bool uBool;
    int uInt;
    uint uUInt; // not supported in API
    float uFloat;
} scalarsUBO;

layout(std140, binding=1) uniform UboTypeVecAndMat
{
    vec2 uVec2f;
    vec3 uVec3f;
    vec4 uVec4f;
    mat2 uMat22f;
    mat3 uMat33f;
    mat4 uMat44f;
} vecMatUBO;

layout(std140, binding=2) uniform UboTypeScalarArrays
{
    bool uBool[5];
    int uInt[7];
    float uFloat[5];
} arraysUBO;

layout(std140, binding=3) uniform UboTypeVecAndMatArrays
{
    vec2 uVec2f[3];
    vec3 uVec3f[3];
    vec4 uVec4f[3];
    mat2 uMat22f[3];
    mat3 uMat33f[3];
    mat4 uMat44f[3];
} vecMatArraysUBO;

struct MyStruct
{
    vec3 uVec3f;
    float uFloat;
    mat3 uMat33f;
    int uInt;
};
layout(std140, binding=4) uniform UboTypeConfidence
{
    float uFloat;
    MyStruct uStruct[2];

} confidenceUBO;

void main()
{
    bool fail = false;

    // scalars
    if(scalarsUBO.uBool != true)
        fail = true;
    if(scalarsUBO.uInt != -111)
        fail = true;
    if(scalarsUBO.uFloat != 333.f)
        fail = true;

    // vector and matrix types
    if(vecMatUBO.uVec2f != vec2(123.0f, 124.0f))
        fail = true;
    if(vecMatUBO.uVec3f != vec3(456.0f, 457.0f, 458.0f))
        fail = true;
    if(vecMatUBO.uVec4f != vec4(678.0f))
        fail = true;
    if(vecMatUBO.uMat22f != mat2(222.0f, 223.0f, 224.0f, 225.0f))
        fail = true;
    if(vecMatUBO.uMat33f != mat3(333.0f, 334.0f, 335.0f, 336.0f, 337.0f, 338.0f, 339.f, 330.f, 331.0f))
        fail = true;
    if(vecMatUBO.uMat44f != mat4(444.0f))
        fail = true;

    // arrays of scalars
    if(arraysUBO.uBool != bool[5](true, false, true, false, true))
        fail = true;
    if(arraysUBO.uInt != int[7](2, 3, 5, 7, 11, 13, 17))
        fail = true;
    if(arraysUBO.uFloat != float[5](33.0f, 66.0f, 99.0f, 0.33f, 0.66f))
        fail = true;

    // arrays of vector and matrix
    if(vecMatArraysUBO.uVec2f != vec2[3](vec2(1.0), vec2(2.0), vec2(3.0)))
        fail = true;
    if(vecMatArraysUBO.uVec3f != vec3[3](vec3(1.0), vec3(2.0), vec3(3.0)))
        fail = true;
    if(vecMatArraysUBO.uVec4f != vec4[3](vec4(1.0), vec4(2.0), vec4(3.0)))
        fail = true;
    if(vecMatArraysUBO.uMat22f != mat2[3](mat2(4.0), mat2(5.0), mat2(6.0)))
        fail = true;
    if(vecMatArraysUBO.uMat33f != mat3[3](mat3(4.0), mat3(5.0), mat3(6.0)))
        fail = true;
    if(vecMatArraysUBO.uMat44f != mat4[3](mat4(4.0), mat4(5.0), mat4(6.0)))
        fail = true;

    // struct array
    if(confidenceUBO.uFloat != 777.0f)
        fail = true;
    if(confidenceUBO.uStruct[0] != MyStruct(vec3(11.0), 12.0, mat3(13.0), 14))
        fail = true;
    if(confidenceUBO.uStruct[1] != MyStruct(vec3(21.0), 22.0, mat3(23.0), 24))
        fail = true;

    if(fail)
        v_fragColor = vec4(1.0f);
    else
        v_fragColor = vec4(1.0f, 0.f, 0.5f, 1.0f);

    gl_Position = mvpMatrix * vec4(a_position, 1.0);
}
