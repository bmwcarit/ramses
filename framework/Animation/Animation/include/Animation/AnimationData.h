//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONDATA_H
#define RAMSES_ANIMATIONDATA_H

#include "AnimationAPI/AnimationSystemSizeInformation.h"
#include "Animation/AnimationDataNotifier.h"
#include "Animation/AnimationCommon.h"
#include "Animation/AnimationInstance.h"
#include "Animation/Spline.h"
#include "Animation/Animation.h"
#include "Animation/AnimationDataBind.h"
#include "Utils/MemoryPool.h"

namespace ramses_internal
{
    struct AnimationProcessData;

    class AnimationData : public AnimationDataNotifier
    {
    public:
        explicit AnimationData(const AnimationSystemSizeInformation& sizeInfo = AnimationSystemSizeInformation());
        virtual ~AnimationData();

        template <template<typename> class Key, typename EDataType>
        SplineHandle                 allocateSpline                      (const Spline<Key, EDataType>& spline, SplineHandle handle = SplineHandle::Invalid());
        template <template<typename> class Key, typename EDataType>
        SplineHandle                 allocateSpline                      (SplineHandle handle = SplineHandle::Invalid());
        const SplineBase*            getSpline                           (SplineHandle splineHandle) const;
        bool                         containsSpline                      (SplineHandle splineHandle) const;
        UInt32                       getTotalSplineCount() const;
        // KeyIndex is not guaranteed to refer to the same key after more keys added due to reordering
        template <template<typename> class Key, typename EDataType>
        SplineKeyIndex               setSplineKey                        (SplineHandle handle, SplineTimeStamp timeStamp, const Key<EDataType>& key);
        void                         removeSplineKey                     (SplineHandle handle, SplineKeyIndex keyIndex);
        void                         removeSplineKeys                    (SplineHandle handle);

        template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
        DataBindHandle               allocateDataBinding                 (const AnimationDataBind<ClassType, EDataType, HandleType, HandleType2>& dataBind, DataBindHandle handle = DataBindHandle::Invalid());
        const AnimationDataBindBase* getDataBinding                      (DataBindHandle dataBindHandle) const;
        AnimationDataBindBase*       getDataBinding                      (DataBindHandle dataBindHandle);
        bool                         containsDataBinding                 (DataBindHandle dataBindHandle) const;
        UInt32                       getTotalDataBindCount               () const;

        AnimationInstanceHandle      allocateAnimationInstance           (SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent = EVectorComponent_All, AnimationInstanceHandle handle = AnimationInstanceHandle::Invalid());
        const AnimationInstance&     getAnimationInstance                (AnimationInstanceHandle instanceHandle) const;
        bool                         containsAnimationInstance           (AnimationInstanceHandle instanceHandle) const;
        UInt32                       getTotalAnimationInstanceCount      () const;
        void                         addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle);

        AnimationHandle              allocateAnimation                   (AnimationInstanceHandle animationInstanceHandle, AnimationHandle handle = AnimationHandle::Invalid());
        const Animation&             getAnimation                        (AnimationHandle handle) const;
        bool                         containsAnimation                   (AnimationHandle handle) const;
        UInt32                       getTotalAnimationCount              () const;
        void                         getAnimationHandles                 (AnimationHandleVector& handles) const;
        void                         setAnimationTimeRange               (AnimationHandle handle, const AnimationTime& startTime, const AnimationTime& stopTime);
        void                         setAnimationPaused                  (AnimationHandle handle, bool pause);
        void                         setAnimationProperties              (AnimationHandle handle, Float playbackSpeed, Animation::Flags flags, AnimationTime::Duration loopDuration);
        void                         getAnimationProcessData             (AnimationHandle handle, AnimationProcessData& processData) const;

        AnimationSystemSizeInformation getTotalSizeInformation           () const;

        void                         removeSpline                        (SplineHandle handle);
        void                         removeDataBinding                   (DataBindHandle handle);
        void                         removeAnimationInstance             (AnimationInstanceHandle handle);
        void                         removeAnimation                     (AnimationHandle handle);

        static bool                  CheckDataTypeCompatibility(EDataTypeID dataTypeID, EDataTypeID accessorDataTypeID, EVectorComponent accessorVectorComponent);

    private:
        AnimationData(const AnimationData&);
        AnimationData& operator=(const AnimationData&);

        AnimationInstance&           getAnimationInstanceInternal(AnimationInstanceHandle handle);
        AnimationInstanceHandle      findAnimationInstance(const AnimationInstance& animationInstance) const;
        bool                         isDataBindingCompatibleWithAnimationInstance(const AnimationInstance& animationInstance, DataBindHandle dataBindHandle) const;
        AnimationHandle              findAnimation(const Animation& animation) const;
        Animation&                   getAnimationInternal(AnimationHandle handle);

        SplineBase*                  getSplineInternal(SplineHandle handle);

        void                         getDataBindingsForAnimationInstance(AnimationInstanceHandle instanceHandle, ConstDataBindVector& dataBinds) const;

        static bool                  isTypeMatchingComponentType(EDataTypeID compType, EDataTypeID type);

        MemoryPool<SplineBase*, SplineHandle>                  m_splinePool;
        MemoryPool<AnimationDataBindBase*, DataBindHandle>     m_dataBindPool;
        MemoryPool<AnimationInstance, AnimationInstanceHandle> m_animationInstancePool;
        MemoryPool<Animation, AnimationHandle>                 m_animationPool;

        friend class AnimationLogic;
    };

    template <template<typename> class Key, typename EDataType>
    SplineHandle AnimationData::allocateSpline(const Spline<Key, EDataType>& spline, SplineHandle handle)
    {
        const SplineHandle handleActual = m_splinePool.allocate(handle);
        *m_splinePool.getMemory(handleActual) = new Spline<Key, EDataType>(spline);
        return handleActual;
    }

    template <template<typename> class Key, typename EDataType>
    SplineHandle AnimationData::allocateSpline(SplineHandle handle)
    {
        const SplineHandle handleActual = m_splinePool.allocate(handle);
        *m_splinePool.getMemory(handleActual) = new Spline<Key, EDataType>;
        return handleActual;
    }

    template <template<typename> class Key, typename EDataType>
    SplineKeyIndex AnimationData::setSplineKey(SplineHandle handle, SplineTimeStamp timeStamp, const Key<EDataType>& key)
    {
        SplineBase* pSpline = getSplineInternal(handle);
        assert(pSpline != nullptr);
        assert(pSpline->getDataType() == DataTypeToDataIDSelector<EDataType>::DataTypeID);
        assert(pSpline->getKeyType() == SplineKeyToSplineKeyIDSelector<Key>::SplineKeyID);

        Spline<Key, EDataType>& spline = static_cast<Spline<Key, EDataType>&>(*pSpline);
        const SplineKeyIndex keyIndex = spline.setKey(timeStamp, key);
        notifySplineChanged(handle);

        return keyIndex;
    }

    template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
    DataBindHandle AnimationData::allocateDataBinding(const AnimationDataBind<ClassType, EDataType, HandleType, HandleType2>& dataBind, DataBindHandle handle)
    {
        const DataBindHandle handleActual = m_dataBindPool.allocate(handle);
        *m_dataBindPool.getMemory(handleActual) = new AnimationDataBind<ClassType, EDataType, HandleType, HandleType2>(dataBind);
        return handleActual;
    }
}

#endif
