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

#ifndef RAMSES_CAPU_WINDOWS_FILESYSTEMITERATOR_H
#define RAMSES_CAPU_WINDOWS_FILESYSTEMITERATOR_H

#include "ramses-capu/os/Generic/FileSystemIterator.h"
#include "ramses-capu/os/File.h"

namespace ramses_capu
{
    namespace os
    {
        class FileSystemIterator : private generic::FileSystemIterator<HANDLE>
        {
        public:
            FileSystemIterator(ramses_capu::File root);
            ~FileSystemIterator();

            ramses_capu::File& operator*();
            ramses_capu::File* operator->();
            bool next();
            bool isValid();

            using generic::FileSystemIterator<HANDLE>::setStepIntoSubdirectories;
        private:

            bool oneLevelUp(bool& found);

            bool stepIntoDirectory(ramses_capu::File directory);

            bool readEntry();

            bool mValid;

            WIN32_FIND_DATAA mFindFileData;
        };

        inline FileSystemIterator::FileSystemIterator(ramses_capu::File root)
            : mValid(false)
        {
            // initialize the search
            stepIntoDirectory(root);
        }

        inline FileSystemIterator::~FileSystemIterator()
        {
            // cleanup remaining open directories
            for (auto& dir : mDirectoryStack)
            {
                FindClose(dir);
            }
        }

        inline bool FileSystemIterator::stepIntoDirectory(ramses_capu::File directory)
        {
            String searchPattern(directory.getPath());
            searchPattern.append("/*.*");
            HANDLE currFileHandle = FindFirstFileA(searchPattern.c_str(), &mFindFileData);
            if (currFileHandle == INVALID_HANDLE_VALUE)
            {
                mValid = false;
                return false;
            }

            mDirectoryStack.push_back(currFileHandle);
            mCurrentDirectory = directory;

            if (StringUtils::Strcmp(mFindFileData.cFileName, ".") == 0 ||
                    StringUtils::Strcmp(mFindFileData.cFileName, "..") == 0)
            {
                // search further
                if (readEntry())
                {
                    mCurrentFile = ramses_capu::File(directory, mFindFileData.cFileName);
                    mValid = true;
                }
                else
                {
                    mCurrentFile = ramses_capu::File("");
                    mValid = false;
                }
            }
            else
            {
                mCurrentFile = ramses_capu::File(directory, mFindFileData.cFileName);
                mValid = true;
            }
            return mValid;
        }

        inline bool FileSystemIterator::oneLevelUp(bool& found)
        {
            if (mDirectoryStack.empty())
            {
                return false;
            }

            // close current directory
            FindClose(mDirectoryStack.back());
            mDirectoryStack.pop_back();

            bool success;
            mCurrentDirectory = mCurrentDirectory.getParentFile(success);

            // read upper directory
            found = readEntry();

            return true;
        }

        inline bool FileSystemIterator::readEntry()
        {
            if (mDirectoryStack.empty())
            {
                // no directory in stack -> nothing to do
                return 0;
            }

            bool found = false;
            do
            {
                HANDLE dir = mDirectoryStack.back();;

                BOOL result = FindNextFileA(dir, &mFindFileData);
                found = result != 0;
            }
            while (found && (StringUtils::Strcmp(mFindFileData.cFileName, ".") == 0 ||
                             StringUtils::Strcmp(mFindFileData.cFileName, "..") == 0));

            return found;
        }

        inline bool FileSystemIterator::next()
        {
            if (!mValid || mDirectoryStack.empty())
            {
                // if current is not valid, next also will be not valid
                mValid = false;
                return false;
            }

            // if current file is a directory and we should traverse
            // sub directories then first go into the directory
            bool found;
            if (mRecurseSubDirectories && mCurrentFile.isDirectory())
            {
                found = stepIntoDirectory(mCurrentFile);
            }
            else
            {
                found = readEntry();
            }

            if (!found)
            {
                // no further file found in this directory -> go up one level
                while (oneLevelUp(found))
                {
                    if (found)
                    {
                        // entry found, stop going up
                        break;
                    }
                }
            }

            if (!found)
            {
                // no further files found -> finished
                mCurrentFile = ramses_capu::File("");
                mValid = false;
            }
            else
            {
                mCurrentFile = ramses_capu::File(mCurrentDirectory, mFindFileData.cFileName);
                mValid = true;
            }
            return mValid;
        }

        inline bool FileSystemIterator::isValid()
        {
            return mValid;
        }

        inline ramses_capu::File& FileSystemIterator::operator*()
        {
            return mCurrentFile;
        }

        inline ramses_capu::File* FileSystemIterator::operator->()
        {
            return &mCurrentFile;
        }
    }
}

#endif //RAMSES_CAPU_WINDOWS_FILESYSTEMITERATOR_H
