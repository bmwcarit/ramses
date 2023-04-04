//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataInt32.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataInt32::DataInt32(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataInt32::~DataInt32()
    {
    }

    status_t DataInt32::setValue(int32_t value)
    {
        return impl.setValue(value);
    }

    status_t DataInt32::getValue(int32_t& value) const
    {
        return impl.getValue(value);
    }
}
