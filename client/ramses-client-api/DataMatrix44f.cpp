//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataMatrix44f.h"
#include "DataObjectImpl.h"

namespace ramses
{
    DataMatrix44f::DataMatrix44f(DataObjectImpl& pimpl)
        : DataObject(pimpl)
    {
    }

    DataMatrix44f::~DataMatrix44f()
    {
    }

    status_t DataMatrix44f::setValue(const float(&matrixElements)[16])
    {
        const ramses_internal::Matrix44f value(matrixElements);
        return impl.setValue(value);
    }

    status_t DataMatrix44f::getValue(float(&matrixElements)[16]) const
    {
        ramses_internal::Matrix44f value;
        const status_t status = impl.getValue(value);

        matrixElements[0] = value.m11;
        matrixElements[1] = value.m12;
        matrixElements[2] = value.m13;
        matrixElements[3] = value.m14;

        matrixElements[4] = value.m21;
        matrixElements[5] = value.m22;
        matrixElements[6] = value.m23;
        matrixElements[7] = value.m24;

        matrixElements[ 8] = value.m31;
        matrixElements[ 9] = value.m32;
        matrixElements[10] = value.m33;
        matrixElements[11] = value.m34;

        matrixElements[12] = value.m41;
        matrixElements[13] = value.m42;
        matrixElements[14] = value.m43;
        matrixElements[15] = value.m44;

        return status;
    }
}
