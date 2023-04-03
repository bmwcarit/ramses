//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataFloat.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataFloat::DataFloat(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataFloat::~DataFloat()
    {
    }

    status_t DataFloat::setValue(float value)
    {
        return impl.setValue(value);
    }

    status_t DataFloat::getValue(float& value) const
    {
        return impl.getValue(value);
    }
}
