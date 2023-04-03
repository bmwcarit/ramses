//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataMatrix33f.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataMatrix33f::DataMatrix33f(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataMatrix33f::~DataMatrix33f()
    {
    }

    status_t DataMatrix33f::setValue(const float(&matrixElements)[9])
    {
        const ramses_internal::Matrix33f value(
            matrixElements[0], matrixElements[1], matrixElements[2],
            matrixElements[3], matrixElements[4], matrixElements[5],
            matrixElements[6], matrixElements[7], matrixElements[8]);
        return impl.setValue(value);
    }

    status_t DataMatrix33f::getValue(float(&matrixElements)[9]) const
    {
        ramses_internal::Matrix33f value;
        const status_t status = impl.getValue(value);

        matrixElements[0] = value.m11;
        matrixElements[1] = value.m12;
        matrixElements[2] = value.m13;

        matrixElements[3] = value.m21;
        matrixElements[4] = value.m22;
        matrixElements[5] = value.m23;

        matrixElements[6] = value.m31;
        matrixElements[7] = value.m32;
        matrixElements[8] = value.m33;

        return status;
    }
}
