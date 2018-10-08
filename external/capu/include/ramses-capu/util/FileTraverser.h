/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_FILETRAVERSER_H
#define RAMSES_CAPU_FILETRAVERSER_H

#include "ramses-capu/os/File.h"
#include "ramses-capu/os/FileSystemIterator.h"
#include "ramses-capu/util/IFileVisitor.h"

namespace ramses_capu
{
    /**
    * Utility class to walk over a file tree using the visitor pattern.
    */
    class FileTraverser
    {
    public:

        /**
        * Starts a new traversal inside the given directory. Will not visit the directory itself.
        * @param directory The directory in which the traversal should start.
        * @param visitor The visitor which get's called for each file or folder inside the given directory.
        * @return Return code if traversal was successful.
        */
        static status_t accept(ramses_capu::File directory, IFileVisitor& visitor);
    };

    inline status_t FileTraverser::accept(ramses_capu::File directory, IFileVisitor& visitor)
    {
        ramses_capu::FileSystemIterator iter(directory);

        status_t result = CAPU_OK;
        while (iter.isValid())
        {
            bool stepIntoDirectory = true;
            result = visitor.visit(*iter, stepIntoDirectory);
            if (result == CAPU_ERROR)
            {
                // user abort
                return result;
            }

            iter.setStepIntoSubdirectories(stepIntoDirectory);
            iter.next();
        }
        return result;
    }
}

#endif // RAMSES_CAPU_FILETRAVERSER_H
