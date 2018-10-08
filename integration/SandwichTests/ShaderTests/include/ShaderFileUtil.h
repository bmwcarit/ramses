//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERFILEUTIL_H
#define RAMSES_SHADERFILEUTIL_H

#include "Collections/Vector.h"
#include "Collections/String.h"

namespace ramses_internal
{
    struct ShaderProgramTestInfo
    {
        String vertexShaderPath;
        String fragmentShaderPath;

        String verboseShaderName;
    };

    class ShaderProgramTestInfoVector : public Vector<ShaderProgramTestInfo>
    {
    public:

        /**
        * Removes shader programs, which do not contain a certain string
        * @param[in] filter string to filter
        */
        void filterIn(const String& filter)
        {
            // collect shader programs to keep
            ShaderProgramTestInfoVector keptShaderPrograms;
            for(UInt32 i = 0; i < size(); i++)
            {
                ShaderProgramTestInfo shaderProgramTestInfo = (*this)[i];
                if (shaderProgramTestInfo.verboseShaderName.find(filter) != -1)
                {
                    keptShaderPrograms.push_back(shaderProgramTestInfo);
                }
            }

            // apply filter
            this->swap(keptShaderPrograms);
        }

    };

    class ShaderFileUtil
    {
    public:

        /**
         * This function returns all shaders from a folder.
         * It looks for *.vert files and checks if there is a corresponding *frag file in the same directory with the same name.
         * It finally returns the relative paths of all found shader program couples in the directory.
         * @param[in] searchFolder directory in which to look for shaders
         * @param[out] shaderProgramTestInfos list of found shader names without extension and relative path names to the passed shader directory
         */
        static void FindShaderPairsWithSameBaseFilenameInDirectory(const String& searchFolder, ShaderProgramTestInfoVector& shaderProgramTestInfos);

    private:
        ShaderFileUtil() {};
    };
}

#endif
