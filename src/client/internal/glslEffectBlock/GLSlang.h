//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/Warnings.h"

#include <cstdint>

WARNINGS_PUSH

WARNING_DISABLE_VC(4100)

WARNING_DISABLE_LINUX(-Wunused-parameter)
WARNING_DISABLE_LINUX(-Wsign-compare)
WARNING_DISABLE_LINUX(-Wold-style-cast)
WARNING_DISABLE_LINUX(-Wshadow)
WARNING_DISABLE_LINUX(-Wreorder)
WARNING_DISABLE_LINUX(-Wnon-virtual-dtor)
WARNING_DISABLE_GCC(-Wsuggest-override)
WARNING_DISABLE_GCC9(-Wdeprecated-copy)

// INCLUDE PROBLEMATIC HEADERS
#include "Include/intermediate.h"
#include "Include/InitializeGlobals.h"
#include "MachineIndependent/localintermediate.h"
#include "OGLCompilersDLL/InitializeDll.h"

WARNINGS_POP

#include "Public/ShaderLang.h"

