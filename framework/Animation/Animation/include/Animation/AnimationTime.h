//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONTIME_H
#define RAMSES_ANIMATIONTIME_H

#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    class AnimationTime
    {
    public:
        typedef UInt64 TimeStamp;
        typedef UInt64 Duration;

        AnimationTime(TimeStamp timeStamp = InvalidTimeStamp);
        AnimationTime(const AnimationTime& other);

        AnimationTime& operator=(const AnimationTime& other) = default;

        Bool isValid() const;
        Duration getDurationSince(const AnimationTime& other) const;
        TimeStamp getTimeStamp() const;

        Bool operator==(const AnimationTime& other) const;
        Bool operator!=(const AnimationTime& other) const;
        Bool operator<(const AnimationTime& other) const;
        Bool operator>(const AnimationTime& other) const;
        Bool operator<=(const AnimationTime& other) const;
        Bool operator>=(const AnimationTime& other) const;
        AnimationTime operator+(const AnimationTime& other) const;
        AnimationTime& operator+=(const AnimationTime& other);

        static const TimeStamp InvalidTimeStamp = TimeStamp(-1);

    private:
        TimeStamp m_time;
    };

    inline AnimationTime::AnimationTime(TimeStamp timeStamp)
        : m_time(timeStamp)
    {
    }

    inline AnimationTime::AnimationTime(const AnimationTime& other)
        : m_time(other.m_time)
    {
    }

    inline Bool AnimationTime::isValid() const
    {
        return m_time != InvalidTimeStamp;
    }

    inline AnimationTime::Duration AnimationTime::getDurationSince(const AnimationTime& other) const
    {
        if (m_time >= other.m_time)
        {
            return m_time - other.m_time;
        }

        return InvalidTimeStamp;
    }

    inline AnimationTime::TimeStamp AnimationTime::getTimeStamp() const
    {
        return m_time;
    }

    inline Bool AnimationTime::operator==(const AnimationTime& other) const
    {
        return m_time == other.m_time;
    }

    inline Bool AnimationTime::operator!=(const AnimationTime& other) const
    {
        return m_time != other.m_time;
    }

    inline Bool AnimationTime::operator<(const AnimationTime& other) const
    {
        return m_time < other.m_time;
    }

    inline Bool AnimationTime::operator>(const AnimationTime& other) const
    {
        return m_time > other.m_time;
    }

    inline Bool AnimationTime::operator<=(const AnimationTime& other) const
    {
        return m_time <= other.m_time;
    }

    inline Bool AnimationTime::operator>=(const AnimationTime& other) const
    {
        return m_time >= other.m_time;
    }

    inline AnimationTime AnimationTime::operator+(const AnimationTime& other) const
    {
        return AnimationTime(m_time + other.m_time);
    }

    inline AnimationTime& AnimationTime::operator+=(const AnimationTime& other)
    {
        m_time += other.m_time;
        return *this;
    }
}

#endif
