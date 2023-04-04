//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataVector4f.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataVector4f::DataVector4f(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataVector4f::~DataVector4f()
    {
    }

    status_t DataVector4f::setValue(const float x, const float y, const float z, const float w)
    {
        const ramses_internal::Vector4 value(x, y, z, w);
        return impl.setValue(value);
    }

    status_t DataVector4f::getValue(float& x, float& y, float& z, float& w) const
    {
        ramses_internal::Vector4 value(x, y, z, w);
        const status_t success = impl.getValue(value);
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;

        return success;
    }
}
