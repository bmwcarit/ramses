//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/File.h"
#include "ramses-capu/os/Console.h"
#include "ShaderConverterTest.h"
#include "ShaderFileUtil.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererConfigUtils.h"

using namespace ramses_internal;

int main(int argc, char** argv)
{
    //parse command line options
    CommandLineParser cmdParser(argc, argv);
    ArgumentString folderArgument(cmdParser, "f", "folder", "");
    ArgumentString filterArgument(cmdParser, "fi", "filterIn", "");
    ArgumentBool noGpuUploadArgument(cmdParser, "ng", "nogpu", false);
    DisplayConfig displayConfig;
    RendererConfigUtils::ApplyValuesFromCommandLine(cmdParser, displayConfig);

    if (folderArgument.wasDefined())
    {
        // create test suite
        ShaderConverterTest testSuite(displayConfig, noGpuUploadArgument);
        if (!noGpuUploadArgument && !testSuite.hasRenderBackend())
        {
            ramses_capu::Console::Print(ramses_capu::Console::RED, "\n\n[ Failed ] No shader program tested: failed to create render backend\n");
            return 1;
        }

        // find shader files
        const String searchFolderPath = String(folderArgument);
        ShaderProgramTestInfoVector shaderProgramTestInfos;
        ShaderFileUtil::FindShaderPairsWithSameBaseFilenameInDirectory(searchFolderPath, shaderProgramTestInfos);
        shaderProgramTestInfos.filterIn(filterArgument);

        ShaderProgramTestInfoVector failedShaderInfos;
        for (UInt32 i = 0; i < shaderProgramTestInfos.size(); ++i)
        {
            const ShaderProgramTestInfo& shaderProgramTestInfo = shaderProgramTestInfos[i];
            ramses_capu::Console::Print(ramses_capu::Console::GREEN, "Check shader program %s (%u / %u)\n", shaderProgramTestInfo.verboseShaderName.c_str(), i + 1, shaderProgramTestInfos.size());

            const Bool shaderSucceeded = testSuite.testShader(shaderProgramTestInfo);

            if (shaderSucceeded)
            {
                ramses_capu::Console::Print(ramses_capu::Console::GREEN, "[ OK ] Shader program %s succeeded\n", shaderProgramTestInfo.verboseShaderName.c_str());
            }
            else
            {
                failedShaderInfos.push_back(shaderProgramTestInfo);

                ramses_capu::Console::Print(ramses_capu::Console::RED, "%s\n [ FAILED ] Shader program %s failed\n", testSuite.getLastErrorMessage(), shaderProgramTestInfo.verboseShaderName.c_str());
            }
        }

        // final test protocol report
        if (noGpuUploadArgument)
        {
            ramses_capu::Console::Print(ramses_capu::Console::YELLOW, "\n[ WARN ] Started with \"--noGPU\" option: GPU shader upload was disabled");
        }
        if (0u == failedShaderInfos.size())
        {
            ramses_capu::Console::Print(ramses_capu::Console::GREEN, "\n[ PASSED ] All shader programs passed the check\n");
            return 0;
        }
        else
        {
            ramses_capu::Console::Print(ramses_capu::Console::RED, "\n[ FAILED ] %u of %u shader programs did not pass the check\n\n", failedShaderInfos.size(), shaderProgramTestInfos.size());

            ramses_capu::Console::Print(ramses_capu::Console::RED, "Failing shader programs:\n");
            ramses_capu::Console::Print(ramses_capu::Console::RED, "------------------------\n");
            for (UInt32 i = 0; i < failedShaderInfos.size(); ++i)
            {
                ramses_capu::Console::Print(ramses_capu::Console::RED, " -> %s\n", failedShaderInfos[i].verboseShaderName.c_str());
            }
            ramses_capu::Console::Print(ramses_capu::Console::RED, "\n");
            return 1;
        }
    }
    else
    {
        ramses_capu::Console::Print(ramses_capu::Console::YELLOW, "Folder parameter not set. Shader converter test will not be executed\n");
        return 1;
    }
}
