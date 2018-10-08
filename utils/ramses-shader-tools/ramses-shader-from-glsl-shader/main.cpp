//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RamsesShaderFromGLSLShaderArguments.h"
#include "ConsoleUtils.h"
#include "ShaderConverter.h"

int main(int argc, char* argv[])
{
    RamsesShaderFromGLSLShaderArguments arguments;
    if (!arguments.loadArguments(argc, argv))
    {
        PRINT("Inputs are not valid! Please find details above ...\n");
        return 1;
    }

    if (!ShaderConverter::Convert(arguments))
    {
        PRINT("Fail! Please find details above...\n");
        return 1;
    }

    PRINT("Succeed!\n");
    return 0;
}
