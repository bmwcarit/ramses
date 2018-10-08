//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINEKEYTANGENTS_H
#define RAMSES_SPLINEKEYTANGENTS_H

#include "Math3d/Vector2.h"

namespace ramses_internal
{
    template <typename EDataType>
    class SplineKeyTangents
    {
    public:
        SplineKeyTangents();
        SplineKeyTangents(const EDataType& value, const Vector2& tangentIn, const Vector2& tangentOut);

        EDataType m_value;
        Vector2 m_tangentIn;
        Vector2 m_tangentOut;
    };

    template <typename EDataType>
    inline SplineKeyTangents<EDataType>::SplineKeyTangents()
        : m_value(0)
        , m_tangentIn(0)
        , m_tangentOut(0)
    {
    }

    template <typename EDataType>
    inline SplineKeyTangents<EDataType>::SplineKeyTangents(const EDataType& value, const Vector2& tangentIn, const Vector2& tangentOut)
        : m_value(value)
        , m_tangentIn(tangentIn)
        , m_tangentOut(tangentOut)
    {
    }
}

#endif
