//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINEBASE_H
#define RAMSES_SPLINEBASE_H

#include "Collections/Vector.h"
#include "Utils/DataTypeUtils.h"
#include "Animation/AnimationCommon.h"
#include "Animation/AnimationCollectionTypes.h"

namespace ramses_internal
{
    class AnimationProcessDataDispatch;

    class SplineBase
    {
    public:
        virtual ~SplineBase() {}
        virtual ESplineKeyType getKeyType() const = 0;
        virtual EDataTypeID getDataType() const = 0;
        virtual void dispatch(AnimationProcessDataDispatch& dispatcher) const = 0;
        virtual void removeKey(SplineKeyIndex keyIndex) = 0;
        virtual void removeAllKeys() = 0;

        UInt32 getNumKeys() const;
        SplineTimeStamp getTimeStamp(const SplineKeyIndex keyIndex) const;

    protected:
        SplineTimeStampVector m_timeStamps;

        SplineKeyIndex getIndexOfKeyAfterOrEqual(const SplineTimeStamp timeStamp) const;
    };

    inline SplineTimeStamp SplineBase::getTimeStamp(const SplineKeyIndex keyIndex) const
    {
        if (keyIndex >= getNumKeys())
            return InvalidSplineTimeStamp;
        return m_timeStamps[keyIndex];
    }

    inline UInt32 SplineBase::getNumKeys() const
    {
        return static_cast<UInt32>(m_timeStamps.size());
    }

    inline SplineKeyIndex SplineBase::getIndexOfKeyAfterOrEqual(const SplineTimeStamp timeStamp) const
    {
        for (SplineKeyIndex index = 0; index < getNumKeys(); ++index)
        {
            if (m_timeStamps[index] >= timeStamp)
            {
                return index;
            }
        }

        return InvalidSplineKeyIndex;
    }
}

#endif
