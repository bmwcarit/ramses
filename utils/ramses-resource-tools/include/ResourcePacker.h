//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCE_TOOLS_RESOURCEPACKER_H
#define RAMSES_RESOURCE_TOOLS_RESOURCEPACKER_H

class RamsesResourcePackerArguments;

class ResourcePacker
{
public:
    static bool Pack(const RamsesResourcePackerArguments& arguments);
};

#endif
