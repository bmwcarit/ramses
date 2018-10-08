//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ShaderFileUtil.h"

#include <ramses-capu/os/File.h>
#include <ramses-capu/os/FileSystemIterator.h>
#include "Collections/String.h"

namespace ramses_internal
{
    void ShaderFileUtil::FindShaderPairsWithSameBaseFilenameInDirectory(const String& searchFolder, ShaderProgramTestInfoVector& shaderProgramTestInfos)
    {
        shaderProgramTestInfos.clear();

        const ramses_capu::File shaderFolderFile(searchFolder);
        const String shaderFolderPath = shaderFolderFile.getPath();

        ramses_capu::FileSystemIterator fileIt(shaderFolderFile);
        fileIt.setStepIntoSubdirectories(true);
        for( ; fileIt.isValid(); fileIt.next())
        {
            ramses_capu::File& currentFile = *fileIt;
            if (currentFile.getExtension() == "vert")
            {
                ShaderProgramTestInfo shaderProgramTestInfo;
                shaderProgramTestInfo.vertexShaderPath = currentFile.getPath().replace("\\", "/");
                shaderProgramTestInfo.fragmentShaderPath = shaderProgramTestInfo.vertexShaderPath.replace("vert", "frag", shaderProgramTestInfo.vertexShaderPath.getLength() - 5u);

                // check if frag file exists
                ramses_capu::File fragShaderFile(shaderProgramTestInfo.fragmentShaderPath);
                if (fragShaderFile.exists())
                {

                    // remove base path and extension
                    UInt shaderFolderPathLength = shaderFolderPath.getLength();
                    // length = full shader program path - shader search folder path - extension (.frag/.vert)
                    UInt verboseShaderNameLength = shaderProgramTestInfo.vertexShaderPath.getLength() - (shaderFolderPathLength + 1u) - 5u;
                    shaderProgramTestInfo.verboseShaderName = shaderProgramTestInfo.vertexShaderPath.substr(shaderFolderPathLength + 1, verboseShaderNameLength);
                    shaderProgramTestInfos.push_back(shaderProgramTestInfo);
                }
            }
        }
    }
}
