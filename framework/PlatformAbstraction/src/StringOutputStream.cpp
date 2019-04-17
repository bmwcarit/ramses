//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/StringOutputStream.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    template <typename MatrixType>
    StringOutputStream& StringOutputStream::outputMatrix(const MatrixType& matrix)
    {
        const UInt32 numElements = sizeof(decltype(matrix.data)) / sizeof(matrix.data[0]);
        for (UInt32 i = 0; i < numElements; i++)
        {
            *this << matrix.data[i];
            if (i < (numElements - 1))
            {
                *this << " ";
            }
        }
        return *this;
    }

    StringOutputStream& StringOutputStream::operator<<(const Matrix22f& value)
    {
        return outputMatrix(value);
    }

    StringOutputStream& StringOutputStream::operator<<(const Matrix33f& value)
    {
        return outputMatrix(value);
    }

    StringOutputStream& StringOutputStream::operator<<(const Matrix44f& value)
    {
        return outputMatrix(value);
    }
}
