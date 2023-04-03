//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataMatrix22f.h"
#include "DataObjectImpl.h"
#include "Math3d/Matrix22f.h"

namespace ramses
{
    DataMatrix22f::DataMatrix22f(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataMatrix22f::~DataMatrix22f()
    {
    }

    status_t DataMatrix22f::setValue(const float(&matrixElements)[4])
    {
        const ramses_internal::Matrix22f value(matrixElements[0], matrixElements[1], matrixElements[2], matrixElements[3]);
        return impl.setValue(value);
    }

    status_t DataMatrix22f::getValue(float(&matrixElements)[4]) const
    {
        ramses_internal::Matrix22f value;
        const status_t status = impl.getValue(value);

        matrixElements[0] = value.m11;
        matrixElements[1] = value.m12;
        matrixElements[2] = value.m21;
        matrixElements[3] = value.m22;

        return status;
    }
}
