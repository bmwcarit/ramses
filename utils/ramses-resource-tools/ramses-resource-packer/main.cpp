//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesResourcePackerArguments.h"
#include "ResourcePacker.h"
#include "ConsoleUtils.h"

int main(int argc, char* argv[])
{
    RamsesResourcePackerArguments arguments;
    if (!arguments.loadArguments(argc, argv))
    {
        PRINT("Inputs are not valid! Please find details above ...\n");
        return 1;
    }

    if (!ResourcePacker::Pack(arguments))
    {
        PRINT("Fail! Please find details above...\n");
        return 1;

    }

    PRINT("Succeed!\n");
    return 0;
}
