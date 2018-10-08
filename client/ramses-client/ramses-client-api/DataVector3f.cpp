//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataVector3f.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataVector3f::DataVector3f(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataVector3f::~DataVector3f()
    {
    }

    status_t DataVector3f::setValue(const float x, const float y, const float z)
    {
        const ramses_internal::Vector3 value(x, y, z);
        return impl.setValue(value);
    }

    status_t DataVector3f::getValue(float& x, float& y, float& z) const
    {
        ramses_internal::Vector3 value;
        status_t success = impl.getValue(value);
        x = value.x;
        y = value.y;
        z = value.z;

        return success;
    }
}
