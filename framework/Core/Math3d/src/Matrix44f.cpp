//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <Math3d/Matrix44f.h>

namespace ramses_internal
{
    const Matrix44f Matrix44f::Identity(  1.f, 0.f, 0.f, 0.f,
                                        0.f, 1.f, 0.f, 0.f,
                                        0.f, 0.f, 1.f, 0.f,
                                        0.f, 0.f, 0.f, 1.f);

    const Matrix44f Matrix44f::Empty( 0.f, 0.f, 0.f, 0.f,
                                    0.f, 0.f, 0.f, 0.f,
                                    0.f, 0.f, 0.f, 0.f,
                                    0.f, 0.f, 0.f, 0.f);
}
