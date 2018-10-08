//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/StringOutputStream.h"
#include "Math3d/Matrix44f.h"
#include "Math3d/Matrix33f.h"

namespace ramses_internal
{
    template <typename MatrixType, UInt32 numberOfElements>
    StringOutputStream& StringOutputStream::outputMatrix(const MatrixType& matrix)
    {
        for (UInt32 i = 0; i < numberOfElements; i++)
        {
            *this << matrix.getRawData()[i];
            if (i < (numberOfElements - 1))
            {
                *this << " ";
            }
        }
        return *this;
    }

    StringOutputStream& StringOutputStream::operator<<(const Matrix44f& value)
    {
        return outputMatrix<Matrix44f, 16>(value);
    }

    StringOutputStream& StringOutputStream::operator<<(const Matrix33f& value)
    {
        return outputMatrix<Matrix33f, 9>(value);
    }
}
