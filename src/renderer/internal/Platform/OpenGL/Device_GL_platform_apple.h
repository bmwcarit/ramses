//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#define LOAD_ALL_API_PROCS(CONTEXT) (void)CONTEXT     // does nothing for apple - api procs are already defined in the GL library
#define DECLARE_ALL_API_PROCS                         // does nothing for apple - api procs are already declared in the GL headers
#define DEFINE_ALL_API_PROCS                          // does nothing for apple - api procs are already defined in the GL headers

// extension procs, however, need to be declared and defined
#define DECLARE_EXTENSION_PROC(TYPE, NAME) extern TYPE NAME;
#define DEFINE_EXTENSION_PROC(TYPE, NAME) TYPE NAME = 0;
