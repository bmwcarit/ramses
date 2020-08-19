//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONPROCESSDATADISPATCH_H
#define RAMSES_ANIMATIONPROCESSDATADISPATCH_H

#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "PlatformAbstraction/VariantWrapper.h"

namespace ramses_internal
{
    struct AnimationProcessData;
    template <template<typename> class Key, typename EDataType>
    class Spline;
    template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
    class AnimationDataBind;

    // Processing dispatcher computes an interpolated value from spline and sets value to destinations via data binding.
    class AnimationProcessDataDispatch
    {
    public:
        explicit AnimationProcessDataDispatch(const AnimationProcessData& processData);

        void dispatch();

        template <template<typename> class Key, typename EDataType>
        void dispatchSpline(const Spline<Key, EDataType>& spline);

        template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
        void dispatchDataBind(const AnimationDataBind<ClassType, EDataType, HandleType, HandleType2>& dataBind) const;

    private:
        const AnimationProcessData& m_processData;
        using Variant = absl::variant<
            absl::monostate,
            bool,
            int32_t,
            int64_t,
            uint32_t,
            uint64_t,
            float,
            double,
            Vector2,
            Vector3,
            Vector4,
            Vector2i,
            Vector3i,
            Vector4i>;
        Variant m_interpolatedValue;

        template <typename ClassType, typename EDataType, typename HandleType, typename HandleType2>
        EDataType getFinalValue(const AnimationDataBind<ClassType, EDataType, HandleType, HandleType2> &dataBind) const;

        template <typename EDataType>
        EDataType getInterpolatedValue(const EDataType& offset) const;
    };

    template <typename EDataType>
    inline EDataType AnimationProcessDataDispatch::getInterpolatedValue(const EDataType& offset) const
    {
        const EDataType& interpolatedValue = absl::get<EDataType>(m_interpolatedValue);
        return offset + interpolatedValue;
    }
}

#endif
