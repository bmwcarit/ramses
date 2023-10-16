//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#ifdef DLT_ENABLED
#include "internal/DltLogAppender/DltAdapterImpl/DltAdapterImpl.h"
using DltAdapter = ramses::internal::DltAdapterImpl;
#else
#include "internal/DltLogAppender/DltAdapterDummy.h"
using DltAdapter = ramses::internal::DltAdapterDummy;
#endif
