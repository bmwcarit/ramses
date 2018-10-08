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

#ifndef RAMSES_CAPU_FILESYSTEMITERATOR_H
#define RAMSES_CAPU_FILESYSTEMITERATOR_H


#include <ramses-capu/Config.h>
#include <ramses-capu/Error.h>
#include <ramses-capu/os/PlatformInclude.h>
#include <ramses-capu/os/File.h>

#include RAMSES_CAPU_PLATFORM_INCLUDE(FileSystemIterator)

namespace ramses_capu
{

    /**
     * Abstract file representation.
     */
    class FileSystemIterator: private ramses_capu::os::FileSystemIterator
    {
    public:

        /**
         * Constructor taking a File object as root.
         *
         * @param root the root element of the iterator
         */
        FileSystemIterator(ramses_capu::File root);

        /**
         * Indirection
         * @return the current file referenced by the iterator
         */
        ramses_capu::File& operator*();

        /**
         * Dereference
         * @return a pointer to the current file the iterator points to
         */
        ramses_capu::File* operator->();

        /**
         * Advances the iterator
         * @return true if the iterator is pointing to an existing file
         *              after advancing, false otherwise
         */
        bool next();

        /**
         * Determines if the iterator is pointing to a valid file.
         * @return true if file is valid, false otherwise
         */
        bool isValid();

        /**
         * Sets or resets the traverse into subdirectory flag.
         * @param value true if the iterator should traverse into subdirectories,
         *              false otherwise
         */
        void setStepIntoSubdirectories(bool value);

    private:

        /**
         * Private copy constructor with no implementation to prevent copying.
         * @param other other iterator
         */
        FileSystemIterator(const FileSystemIterator& other);

        /**
         * Private assignment operator with no implementation to prevent assignment.
         * @param other other iterator
         * @return reference to this
         */
        FileSystemIterator& operator=(const FileSystemIterator& other);
    };


    inline FileSystemIterator::FileSystemIterator(ramses_capu::File root)
        : ramses_capu::os::FileSystemIterator(root)
    {
    }

    inline bool FileSystemIterator::isValid()
    {
        return ramses_capu::os::FileSystemIterator::isValid();
    }

    inline bool FileSystemIterator::next()
    {
        return ramses_capu::os::FileSystemIterator::next();
    }

    inline ramses_capu::File& FileSystemIterator::operator*()
    {
        return ramses_capu::os::FileSystemIterator::operator*();
    }

    inline ramses_capu::File* FileSystemIterator::operator->()
    {
        return ramses_capu::os::FileSystemIterator::operator->();
    }

    inline void FileSystemIterator::setStepIntoSubdirectories(bool value)
    {
        ramses_capu::os::FileSystemIterator::setStepIntoSubdirectories(value);
    }

}


#endif //RAMSES_CAPU_FILESYSTEMITERATOR_H
