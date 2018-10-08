//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DLTADAPTER_H
#define RAMSES_DLTADAPTER_H

#ifdef DLT_ENABLED
#include "DltLogAppender/DltAdapterImpl/DltAdapterImpl.h"
using DltAdapter = ramses_internal::DltAdapterImpl;
#else
#include "DltLogAppender/DltAdapterDummy/DltAdapterDummy.h"
using DltAdapter = ramses_internal::DltAdapterDummy;
#endif

#endif // RAMSES_DLTADAPTER_H
