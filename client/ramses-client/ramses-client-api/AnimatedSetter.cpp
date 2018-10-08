//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/AnimatedSetter.h"

// internal
#include "AnimatedSetterImpl.h"

namespace ramses
{
    AnimatedSetter::AnimatedSetter(AnimatedSetterImpl& pimpl)
        : AnimationObject(pimpl)
        , impl(pimpl)
    {
    }

    AnimatedSetter::~AnimatedSetter()
    {
    }

    status_t AnimatedSetter::setValue(bool x)
    {
        const status_t status = impl.setValue(x);
        LOG_HL_CLIENT_API1(status, x);
        return status;
    }

    status_t AnimatedSetter::setValue(int32_t x)
    {
        const status_t status = impl.setValue(x);
        LOG_HL_CLIENT_API1(status, x);
        return status;
    }

    status_t AnimatedSetter::setValue(float x)
    {
        const status_t status = impl.setValue(x);
        LOG_HL_CLIENT_API1(status, x);
        return status;
    }

    status_t AnimatedSetter::setValue(float x, float y)
    {
        const status_t status = impl.setValue(x, y);
        LOG_HL_CLIENT_API2(status, x, y);
        return status;
    }

    status_t AnimatedSetter::setValue(float x, float y, float z)
    {
        const status_t status = impl.setValue(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    status_t AnimatedSetter::setValue(float x, float y, float z, float w)
    {
        const status_t status = impl.setValue(x, y, z, w);
        LOG_HL_CLIENT_API4(status, x, y, z, w);
        return status;
    }

    status_t AnimatedSetter::setValue(int32_t x, int32_t y)
    {
        const status_t status = impl.setValue(x, y);
        LOG_HL_CLIENT_API2(status, x, y);
        return status;
    }

    status_t AnimatedSetter::setValue(int32_t x, int32_t y, int32_t z)
    {
        const status_t status = impl.setValue(x, y, z);
        LOG_HL_CLIENT_API3(status, x, y, z);
        return status;
    }

    status_t AnimatedSetter::setValue(int32_t x, int32_t y, int32_t z, int32_t w)
    {
        const status_t status = impl.setValue(x, y, z, w);
        LOG_HL_CLIENT_API4(status, x, y, z, w);
        return status;
    }

    status_t AnimatedSetter::stop(timeMilliseconds_t delay)
    {
        const status_t status = impl.stop(delay);
        LOG_HL_CLIENT_API1(status, delay);
        return status;
    }
}
