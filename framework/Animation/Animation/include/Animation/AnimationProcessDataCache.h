//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONPROCESSDATACACHE_H
#define RAMSES_ANIMATIONPROCESSDATACACHE_H

#include "Collections/HashMap.h"
#include "Animation/AnimationProcessData.h"
#include "Animation/AnimationData.h"

namespace ramses_internal
{
    class AnimationProcessDataCache
    {
    public:
        using DataProcessMap = HashMap<AnimationHandle, AnimationProcessData>;

        explicit AnimationProcessDataCache(const AnimationData& animationData);

        void addProcessData(AnimationHandle handle);
        void removeProcessData(AnimationHandle handle);
        bool hasProcessData(AnimationHandle handle) const;

        DataProcessMap::ConstIterator begin() const;
        DataProcessMap::Iterator begin();
        DataProcessMap::ConstIterator end() const;
        DataProcessMap::Iterator end();

    private:
        void addProcessDataFor(AnimationHandle handle);

        DataProcessMap m_processDataCache;
        const AnimationData& m_animationData;
    };

    inline AnimationProcessDataCache::AnimationProcessDataCache(const AnimationData& animationData)
        : m_animationData(animationData)
    {
    }

    inline void AnimationProcessDataCache::addProcessData(AnimationHandle handle)
    {
        if (!hasProcessData(handle))
        {
            addProcessDataFor(handle);
        }
    }

    inline void AnimationProcessDataCache::removeProcessData(AnimationHandle handle)
    {
        m_processDataCache.remove(handle);
    }

    inline bool AnimationProcessDataCache::hasProcessData(AnimationHandle handle) const
    {
        return ( m_processDataCache.find(handle) != m_processDataCache.end() );
    }

    inline AnimationProcessDataCache::DataProcessMap::ConstIterator AnimationProcessDataCache::begin() const
    {
        return m_processDataCache.begin();
    }

    inline AnimationProcessDataCache::DataProcessMap::Iterator AnimationProcessDataCache::begin()
    {
        return m_processDataCache.begin();
    }

    inline AnimationProcessDataCache::DataProcessMap::ConstIterator AnimationProcessDataCache::end() const
    {
        return m_processDataCache.end();
    }

    inline AnimationProcessDataCache::DataProcessMap::Iterator AnimationProcessDataCache::end()
    {
        return m_processDataCache.end();
    }

    inline void AnimationProcessDataCache::addProcessDataFor(AnimationHandle handle)
    {
        AnimationProcessData processData;
        m_animationData.getAnimationProcessData(handle, processData);
        m_processDataCache.put(handle, processData);
    }
}

#endif
