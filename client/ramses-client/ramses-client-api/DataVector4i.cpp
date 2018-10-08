//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataVector4i.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataVector4i::DataVector4i(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataVector4i::~DataVector4i()
    {
    }

    status_t DataVector4i::setValue(const int32_t x, const int32_t y, const int32_t z, const int32_t w)
    {
        const ramses_internal::Vector4i value(x, y, z, w);
        return impl.setValue(value);
    }

    status_t DataVector4i::getValue(int32_t& x, int32_t& y, int32_t& z, int32_t& w) const
    {
        ramses_internal::Vector4i value;
        status_t success = impl.getValue(value);
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;

        return success;
    }
}
