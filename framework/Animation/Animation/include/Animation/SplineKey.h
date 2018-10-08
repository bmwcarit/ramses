//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINEKEY_H
#define RAMSES_SPLINEKEY_H

namespace ramses_internal
{
    template <typename EDataType>
    class SplineKey
    {
    public:
        SplineKey();
        explicit SplineKey(const EDataType& value);

        EDataType m_value;
    };

    template <typename EDataType>
    inline SplineKey<EDataType>::SplineKey()
        : m_value(0)
    {
    }

    template <typename EDataType>
    inline SplineKey<EDataType>::SplineKey(const EDataType& value)
        : m_value(value)
    {
    }
}

#endif
