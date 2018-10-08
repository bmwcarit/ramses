//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APIEXPORT_H
#define RAMSES_APIEXPORT_H

#ifndef RAMSES_API

#if defined(_WIN32)
    #define RAMSES_API_EXPORT __declspec(dllexport)
    #define RAMSES_API_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    #define RAMSES_API_EXPORT __attribute((visibility ("default")))
    #define RAMSES_API_IMPORT
#elif defined(__ghs__)
    // not needed, does not use shared library
    #define RAMSES_API_EXPORT
    #define RAMSES_API_IMPORT
#else
    #error "Compiler unknown/not supported"
#endif

#if defined(RAMSES_LINK_SHARED_EXPORT)
    #define RAMSES_API RAMSES_API_EXPORT
#elif defined(RAMSES_LINK_SHARED_IMPORT)
    #define RAMSES_API RAMSES_API_IMPORT
#elif defined(RAMSES_LINK_STATIC)
    #define RAMSES_API
#else
    #define RAMSES_API RAMSES_API_IMPORT
#endif

#endif

#endif
