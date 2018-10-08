//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataVector2f.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataVector2f::DataVector2f(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataVector2f::~DataVector2f()
    {
    }

    status_t DataVector2f::setValue(const float x, const float y)
    {
        const ramses_internal::Vector2 value(x, y);
        return impl.setValue(value);
    }

    status_t DataVector2f::getValue(float& x, float& y) const
    {
        ramses_internal::Vector2 value;
        status_t success = impl.getValue(value);
        x = value.x;
        y = value.y;

        return success;
    }
}
