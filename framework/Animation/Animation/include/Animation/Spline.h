//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATION_SPLINE_H
#define RAMSES_ANIMATION_SPLINE_H

#include "Animation/AnimationCommon.h"
#include "Animation/SplineBase.h"
#include "Animation/AnimationProcessDataDispatch.h"

namespace ramses_internal
{
    template <template<typename> class Key, typename EDataType>
    class Spline : public SplineBase
    {
    public:
        typedef Spline<Key, EDataType> SplineType;
        typedef Key<EDataType> KeyType;

        // SplineBase interface
        virtual ESplineKeyType getKeyType() const override;
        virtual EDataTypeID getDataType() const override;
        virtual void dispatch(AnimationProcessDataDispatch& dispatcher) const override;
        virtual void removeKey(SplineKeyIndex keyIndex) override;
        virtual void removeAllKeys() override;

        // KeyIndex is not guaranteed to refer to the same key after more keys added due to reordering
        SplineKeyIndex setKey(SplineTimeStamp timeStamp, const KeyType& key);
        const KeyType& getKey(SplineKeyIndex keyIndex) const;
        KeyType& getKey(SplineKeyIndex keyIndex);

    private:
        void appendKey(SplineTimeStamp timeStamp, const KeyType& key);
        void setKeyInternal(SplineKeyIndex keyIndex, SplineTimeStamp timeStamp, const KeyType& key);
        void shiftKeysByOneStartingFrom(SplineKeyIndex startFromIndex);

        typedef std::vector<KeyType> KeyVector;

        KeyVector m_keys;
    };

    template <template<typename> class Key, typename EDataType>
    inline ESplineKeyType Spline<Key, EDataType>::getKeyType() const
    {
        return SplineKeyToSplineKeyIDSelector<Key>::SplineKeyID;
    }

    template <template<typename> class Key, typename EDataType>
    inline EDataTypeID Spline<Key, EDataType>::getDataType() const
    {
        return DataTypeToDataIDSelector<EDataType>::DataTypeID;
    }

    template <template<typename> class Key, typename EDataType>
    inline void Spline<Key, EDataType>::dispatch(AnimationProcessDataDispatch& dispatcher) const
    {
        dispatcher.dispatchSpline(*this);
    }

    template <template<typename> class Key, typename EDataType>
    inline SplineKeyIndex Spline<Key, EDataType>::setKey(SplineTimeStamp timeStamp, const KeyType& key)
    {
        const SplineKeyIndex spotIndex = getIndexOfKeyAfterOrEqual(timeStamp);
        if (spotIndex == InvalidSplineKeyIndex)
        {
            appendKey(timeStamp, key);
            return static_cast<UInt32>(m_timeStamps.size() - 1u);
        }

        if (m_timeStamps[spotIndex] == timeStamp)
        {
            setKeyInternal(spotIndex, timeStamp, key);
        }
        else
        {
            KeyType dummyKey;
            appendKey(timeStamp, dummyKey);
            shiftKeysByOneStartingFrom(spotIndex);
            setKeyInternal(spotIndex, timeStamp, key);
        }

        return spotIndex;
    }

    template <template<typename> class Key, typename EDataType>
    inline const Key<EDataType>& Spline<Key, EDataType>::getKey(SplineKeyIndex keyIndex) const
    {
        assert(keyIndex < getNumKeys());
        return m_keys[keyIndex];
    }

    template <template<typename> class Key, typename EDataType>
    inline Key<EDataType>& Spline<Key, EDataType>::getKey(SplineKeyIndex keyIndex)
    {
        // Use const version of GetKey to avoid code duplication
        return const_cast<Key<EDataType>&>(const_cast<const Spline<Key, EDataType>&>(*this).getKey(keyIndex));
    }

    template <template<typename> class Key, typename EDataType>
    inline void Spline<Key, EDataType>::removeKey(SplineKeyIndex keyIndex)
    {
        if (keyIndex < getNumKeys())
        {
            m_timeStamps.erase(m_timeStamps.begin() + keyIndex);
            m_keys.erase(m_keys.begin() + keyIndex);
        }
    }

    template <template<typename> class Key, typename EDataType>
    inline void Spline<Key, EDataType>::removeAllKeys()
    {
        m_timeStamps.clear();
        m_keys.clear();
    }

    template <template<typename> class Key, typename EDataType>
    inline void Spline<Key, EDataType>::appendKey(SplineTimeStamp timeStamp, const Key<EDataType>& key)
    {
        m_timeStamps.push_back(timeStamp);
        m_keys.push_back(key);
    }

    template <template<typename> class Key, typename EDataType>
    inline void Spline<Key, EDataType>::setKeyInternal(SplineKeyIndex keyIndex, SplineTimeStamp timeStamp, const Key<EDataType>& key)
    {
        assert(keyIndex < getNumKeys());
        m_timeStamps[keyIndex] = timeStamp;
        m_keys[keyIndex] = key;
    }

    template <template<typename> class Key, typename EDataType>
    inline void Spline<Key, EDataType>::shiftKeysByOneStartingFrom(SplineKeyIndex startFromIndex)
    {
        Int32 numKeys = static_cast<Int32>(m_timeStamps.size());
        for (Int32 i = numKeys - 2; i >= static_cast<Int32>(startFromIndex); --i)
        {
            m_timeStamps[i + 1] = m_timeStamps[i];
            m_keys[i + 1] = m_keys[i];
        }
    }
}

#endif
