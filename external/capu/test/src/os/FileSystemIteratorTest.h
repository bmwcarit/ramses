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

#ifndef RAMSES_CAPU_OS_FILESYSTEMITERATORTEST_H
#define RAMSES_CAPU_OS_FILESYSTEMITERATORTEST_H

#include "gmock/gmock.h"
#include "ramses-capu/os/File.h"
#include "ramses-capu/container/vector.h"
#include <memory>

namespace ramses_capu
{
    class TestFile;

    class TestDirectory;

    typedef std::shared_ptr<TestFile> TestFilePtr;
    typedef std::shared_ptr<TestDirectory> TestDirectoryPtr;

    class TestFile
    {
    public:
        TestFile(File file);
        ~TestFile();

        File getFile()
        {
            return mFile;
        }

    private:
        File mFile;
        bool mCreated;
    };

    class TestDirectory
    {
    public:
        TestDirectory(File dir);

        ~TestDirectory();

        TestDirectory* addFile(String name)
        {
            TestFilePtr file(new TestFile(File(mFile, name)));
            mContainingFiles.push_back(file);
            return this;
        }

        TestDirectoryPtr addDirectory(String name)
        {
            TestDirectoryPtr dir(new TestDirectory(File(mFile, name)));
            mContainingDirectories.push_back(dir);
            return dir;
        }

        File getFile()
        {
            return mFile;
        }

    private:
        File mFile;
        bool mCreated;

        vector<TestFilePtr> mContainingFiles;
        vector<TestDirectoryPtr> mContainingDirectories;
    };

    inline TestFile::TestFile(File file)
        : mFile(file), mCreated(false)
    {
        if (!mFile.exists())
        {
            mFile.createFile();
            mCreated = true;
        }
    }

    inline TestFile::~TestFile()
    {
        if (mCreated)
        {
            mFile.remove();
        }
    }

    inline TestDirectory::TestDirectory(File dir)
        : mFile(dir), mCreated(false)
    {
        if (!mFile.exists())
        {
            mCreated = true;
            mFile.createDirectory();
        }
    }

    inline TestDirectory::~TestDirectory()
    {
        mContainingFiles.clear();
        mContainingDirectories.clear();

        if (mCreated)
        {
            mFile.remove();
        }
    }
}

#endif // RAMSES_CAPU_OS_FILESYSTEMITERATORTEST_H
