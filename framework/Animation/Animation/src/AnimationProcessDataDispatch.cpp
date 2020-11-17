//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationProcessDataDispatch.h"
#include "Animation/SplineBase.h"
#include "Animation/SplineSolver.h"
#include "Animation/AnimationDataBind.h"
#include "Animation/AnimationProcessData.h"
#include "Animation/AnimatableTypeTraits.h"
#include "SceneAPI/IScene.h"

namespace ramses_internal
{
    AnimationProcessDataDispatch::AnimationProcessDataDispatch(const AnimationProcessData& processData)
        : m_processData(processData)
    {
    }

    void AnimationProcessDataDispatch::dispatch()
    {
        m_processData.m_spline->dispatch(*this);

        for (const auto dataBind : m_processData.m_dataBinds)
        {
            assert(dataBind != nullptr);
            dataBind->dispatch(*this);
        }
    }

    template <template<typename> class Key, typename EDataType>
    void AnimationProcessDataDispatch::dispatchSpline(const Spline<Key, EDataType>& spline)
    {
        const SplineSolver<Key, EDataType> splineSolver(spline, m_processData.m_splineIterator, m_processData.m_interpolationType);
        m_interpolatedValue = splineSolver.getInterpolatedValue();
    }

    template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
    void AnimationProcessDataDispatch::dispatchDataBind(const AnimationDataBind<ClassType, EDataType, HandleType, HandleType2>& dataBind) const
    {
        const bool setInitialValue = (m_processData.m_animation.m_flags & Animation::EAnimationFlags_ApplyInitialValue) != 0;
        const EDataType finalValue = ( setInitialValue ? dataBind.getInitialValue() : getFinalValue(dataBind) );
        dataBind.setValue(finalValue);
    }

    template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
    EDataType AnimationProcessDataDispatch::getFinalValue(const AnimationDataBind<ClassType, EDataType, HandleType, HandleType2> &dataBind) const
    {
        const bool relativeAnim = (m_processData.m_animation.m_flags & Animation::EAnimationFlags_Relative) != 0;
        const EDataType offsetValue = (relativeAnim ? dataBind.getInitialValue() : EDataType(0));
        EDataType finalValue;

        if (m_processData.m_dataComponent == EVectorComponent_All)
        {
            finalValue = getInterpolatedValue(offsetValue);
        }
        else
        {
            using TypeTraits = AnimatableTypeTraits<EDataType>;
            const typename TypeTraits::ComponentType offsetComponentValue = TypeTraits::GetComponent(offsetValue, m_processData.m_dataComponent);
            const typename TypeTraits::ComponentType finalComponentValue = getInterpolatedValue(offsetComponentValue);
            finalValue = dataBind.getValue();
            TypeTraits::SetComponent(finalValue, finalComponentValue, m_processData.m_dataComponent);
        }

        return finalValue;
    }

    template <>
    bool AnimationProcessDataDispatch::getInterpolatedValue(const bool& offset) const
    {
        const bool interpolatedValue = absl::get<bool>(m_interpolatedValue);
        return offset ^ interpolatedValue;
    }

    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, bool>(const Spline<SplineKey, bool>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Int32>(const Spline<SplineKey, Int32>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Int64>(const Spline<SplineKey, Int64>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, UInt32>(const Spline<SplineKey, UInt32>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, UInt64>(const Spline<SplineKey, UInt64>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Float>(const Spline<SplineKey, Float>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Double>(const Spline<SplineKey, Double>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Vector2>(const Spline<SplineKey, Vector2>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Vector3>(const Spline<SplineKey, Vector3>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Vector4>(const Spline<SplineKey, Vector4>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Vector2i>(const Spline<SplineKey, Vector2i>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Vector3i>(const Spline<SplineKey, Vector3i>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKey, Vector4i>(const Spline<SplineKey, Vector4i>&);

    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, bool>(const Spline<SplineKeyTangents, bool>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Int32>(const Spline<SplineKeyTangents, Int32>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Int64>(const Spline<SplineKeyTangents, Int64>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, UInt32>(const Spline<SplineKeyTangents, UInt32>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, UInt64>(const Spline<SplineKeyTangents, UInt64>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Float>(const Spline<SplineKeyTangents, Float>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Double>(const Spline<SplineKeyTangents, Double>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Vector2>(const Spline<SplineKeyTangents, Vector2>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Vector3>(const Spline<SplineKeyTangents, Vector3>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Vector4>(const Spline<SplineKeyTangents, Vector4>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Vector2i>(const Spline<SplineKeyTangents, Vector2i>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Vector3i>(const Spline<SplineKeyTangents, Vector3i>&);
    template void AnimationProcessDataDispatch::dispatchSpline<SplineKeyTangents, Vector4i>(const Spline<SplineKeyTangents, Vector4i>&);

    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, bool   >(const AnimationDataBind<IScene, bool   >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int32  >(const AnimationDataBind<IScene, Int32  >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int64  >(const AnimationDataBind<IScene, Int64  >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt32 >(const AnimationDataBind<IScene, UInt32 >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt64 >(const AnimationDataBind<IScene, UInt64 >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Float  >(const AnimationDataBind<IScene, Float  >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Double >(const AnimationDataBind<IScene, Double >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2>(const AnimationDataBind<IScene, Vector2>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3>(const AnimationDataBind<IScene, Vector3>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4>(const AnimationDataBind<IScene, Vector4>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2i>(const AnimationDataBind<IScene, Vector2i>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3i>(const AnimationDataBind<IScene, Vector3i>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4i>(const AnimationDataBind<IScene, Vector4i>&) const;

    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, bool   , MemoryHandle>(const AnimationDataBind<IScene, bool   , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int32  , MemoryHandle>(const AnimationDataBind<IScene, Int32  , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int64  , MemoryHandle>(const AnimationDataBind<IScene, Int64  , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt32 , MemoryHandle>(const AnimationDataBind<IScene, UInt32 , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt64 , MemoryHandle>(const AnimationDataBind<IScene, UInt64 , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Float  , MemoryHandle>(const AnimationDataBind<IScene, Float  , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Double , MemoryHandle>(const AnimationDataBind<IScene, Double , MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2, MemoryHandle>(const AnimationDataBind<IScene, Vector2, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3, MemoryHandle>(const AnimationDataBind<IScene, Vector3, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4, MemoryHandle>(const AnimationDataBind<IScene, Vector4, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2i, MemoryHandle>(const AnimationDataBind<IScene, Vector2i, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3i, MemoryHandle>(const AnimationDataBind<IScene, Vector3i, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4i, MemoryHandle>(const AnimationDataBind<IScene, Vector4i, MemoryHandle>&) const;

    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, bool,    MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, bool,    MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int32,   MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Int32,   MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int64,   MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Int64,   MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt32,  MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, UInt32,  MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt64,  MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, UInt64,  MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Float,   MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Float,   MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Double,  MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Double,  MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2, MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Vector2, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3, MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Vector3, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4, MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Vector4, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2i, MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Vector2i, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3i, MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Vector3i, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4i, MemoryHandle, MemoryHandle>(const AnimationDataBind<IScene, Vector4i, MemoryHandle, MemoryHandle>&) const;

    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, bool   , NodeHandle>(const AnimationDataBind<IScene, bool   , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int32  , NodeHandle>(const AnimationDataBind<IScene, Int32  , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Int64  , NodeHandle>(const AnimationDataBind<IScene, Int64  , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt32 , NodeHandle>(const AnimationDataBind<IScene, UInt32 , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, UInt64 , NodeHandle>(const AnimationDataBind<IScene, UInt64 , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Float  , NodeHandle>(const AnimationDataBind<IScene, Float  , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Double , NodeHandle>(const AnimationDataBind<IScene, Double , NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2, NodeHandle>(const AnimationDataBind<IScene, Vector2, NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3, NodeHandle>(const AnimationDataBind<IScene, Vector3, NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4, NodeHandle>(const AnimationDataBind<IScene, Vector4, NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector2i, NodeHandle>(const AnimationDataBind<IScene, Vector2i, NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector3i, NodeHandle>(const AnimationDataBind<IScene, Vector3i, NodeHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind<IScene, Vector4i, NodeHandle>(const AnimationDataBind<IScene, Vector4i, NodeHandle>&) const;
}
