//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINETESTMACROS_H
#define RAMSES_SPLINETESTMACROS_H

#define CHECK_SET_GET_KEY1(_spline, _componentType, _keyIdx, _timeStamp, _val1) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1)); \
    _componentType val1actual = _componentType(0); \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
}

#define CHECK_SET_GET_KEY2(_spline, _componentType, _keyIdx, _timeStamp, _val1, _val2) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _val2)); \
    _componentType val1actual = _componentType(0); \
    _componentType val2actual = _componentType(0); \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, val2actual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_val2, val2actual); \
}

#define CHECK_SET_GET_KEY3(_spline, _componentType, _keyIdx, _timeStamp, _val1, _val2, _val3) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _val2, _val3)); \
    _componentType val1actual = _componentType(0); \
    _componentType val2actual = _componentType(0); \
    _componentType val3actual = _componentType(0); \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, val2actual, val3actual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_val2, val2actual); \
    EXPECT_EQ(_val3, val3actual); \
}

#define CHECK_SET_GET_KEY4(_spline, _componentType, _keyIdx, _timeStamp, _val1, _val2, _val3, _val4) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _val2, _val3, _val4)); \
    _componentType val1actual = _componentType(0); \
    _componentType val2actual = _componentType(0); \
    _componentType val3actual = _componentType(0); \
    _componentType val4actual = _componentType(0); \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, val2actual, val3actual, val4actual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_val2, val2actual); \
    EXPECT_EQ(_val3, val3actual); \
    EXPECT_EQ(_val4, val4actual); \
}

#define CHECK_SET_GET_KEY_TANGENTS1(_spline, _componentType, _keyIdx, _timeStamp, _val1, _tan1, _tan2) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _tan1.x, _tan1.y, _tan2.x, _tan2.y)); \
    _componentType val1actual = _componentType(0); \
    float tan1xActual = 0.f; \
    float tan1yActual = 0.f; \
    float tan2xActual = 0.f; \
    float tan2yActual = 0.f; \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, tan1xActual, tan1yActual, tan2xActual, tan2yActual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_tan1.x, tan1xActual); \
    EXPECT_EQ(_tan1.y, tan1yActual); \
    EXPECT_EQ(_tan2.x, tan2xActual); \
    EXPECT_EQ(_tan2.y, tan2yActual); \
}

#define CHECK_SET_GET_KEY_TANGENTS2(_spline, _componentType, _keyIdx, _timeStamp, _val1, _val2, _tan1, _tan2) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _val2, _tan1.x, _tan1.y, _tan2.x, _tan2.y)); \
    _componentType val1actual = _componentType(0); \
    _componentType val2actual = _componentType(0); \
    float tan1xActual = 0.f; \
    float tan1yActual = 0.f; \
    float tan2xActual = 0.f; \
    float tan2yActual = 0.f; \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, val2actual, tan1xActual, tan1yActual, tan2xActual, tan2yActual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_val2, val2actual); \
    EXPECT_EQ(_tan1.x, tan1xActual); \
    EXPECT_EQ(_tan1.y, tan1yActual); \
    EXPECT_EQ(_tan2.x, tan2xActual); \
    EXPECT_EQ(_tan2.y, tan2yActual); \
}

#define CHECK_SET_GET_KEY_TANGENTS3(_spline, _componentType, _keyIdx, _timeStamp, _val1, _val2, _val3, _tan1, _tan2) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _val2, _val3, _tan1.x, _tan1.y, _tan2.x, _tan2.y)); \
    _componentType val1actual = _componentType(0); \
    _componentType val2actual = _componentType(0); \
    _componentType val3actual = _componentType(0); \
    float tan1xActual = 0.f; \
    float tan1yActual = 0.f; \
    float tan2xActual = 0.f; \
    float tan2yActual = 0.f; \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, val2actual, val3actual, tan1xActual, tan1yActual, tan2xActual, tan2yActual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_val2, val2actual); \
    EXPECT_EQ(_val3, val3actual); \
    EXPECT_EQ(_tan1.x, tan1xActual); \
    EXPECT_EQ(_tan1.y, tan1yActual); \
    EXPECT_EQ(_tan2.x, tan2xActual); \
    EXPECT_EQ(_tan2.y, tan2yActual); \
}

#define CHECK_SET_GET_KEY_TANGENTS4(_spline, _componentType, _keyIdx, _timeStamp, _val1, _val2, _val3, _val4, _tan1, _tan2) \
{ \
    EXPECT_EQ(StatusOK, _spline.setKey(_timeStamp, _val1, _val2, _val3, _val4, _tan1.x, _tan1.y, _tan2.x, _tan2.y)); \
    _componentType val1actual = _componentType(0); \
    _componentType val2actual = _componentType(0); \
    _componentType val3actual = _componentType(0); \
    _componentType val4actual = _componentType(0); \
    float tan1xActual = 0.f; \
    float tan1yActual = 0.f; \
    float tan2xActual = 0.f; \
    float tan2yActual = 0.f; \
    splineTimeStamp_t timeStampActual = 0u; \
    EXPECT_EQ(StatusOK, _spline.getKeyValues(_keyIdx, timeStampActual, val1actual, val2actual, val3actual, val4actual, tan1xActual, tan1yActual, tan2xActual, tan2yActual)); \
    EXPECT_EQ(_timeStamp, timeStampActual); \
    EXPECT_EQ(_val1, val1actual); \
    EXPECT_EQ(_val2, val2actual); \
    EXPECT_EQ(_val3, val3actual); \
    EXPECT_EQ(_val4, val4actual); \
    EXPECT_EQ(_tan1.x, tan1xActual); \
    EXPECT_EQ(_tan1.y, tan1yActual); \
    EXPECT_EQ(_tan2.x, tan2xActual); \
    EXPECT_EQ(_tan2.y, tan2yActual); \
}

#endif
