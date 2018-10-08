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

#include <gtest/gtest.h>
#include "ramses-capu/Config.h"
#include "ramses-capu/os/File.h"
#include "ramses-capu/os/FileSystemIterator.h"
#include "ramses-capu/container/HashSet.h"

#include "FileSystemIteratorTest.h"

using namespace ramses_capu;

TEST(FileSystemIterator, defaultAcceptTest)
{
    // setup
    TestDirectory rootDir(File("foobarfolder"));
    rootDir.addFile("foobar1.txt");
    rootDir.addDirectory("foobar")->addFile("foobar2.txt");


    HashSet<String> filenames;
    filenames.put("foobar1.txt");
    filenames.put("foobar");
    filenames.put("foobar2.txt");

    FileSystemIterator iter(rootDir.getFile());

    EXPECT_TRUE(iter.isValid());
    EXPECT_TRUE(filenames.hasElement(iter->getFileName()));
    filenames.remove(iter->getFileName());

    EXPECT_TRUE(iter.next());
    EXPECT_TRUE(filenames.hasElement(iter->getFileName()));
    filenames.remove(iter->getFileName());

    EXPECT_TRUE(iter.next());
    EXPECT_TRUE(filenames.hasElement(iter->getFileName()));
    filenames.remove(iter->getFileName());

    EXPECT_EQ(0u, filenames.count());

    EXPECT_FALSE(iter.next());
    EXPECT_FALSE(iter.isValid());


    // iterate again, but don't finish
    FileSystemIterator iter2(rootDir.getFile());
}

TEST(FileSystemIterator, oneFile)
{
    TestDirectory rootDir(File("foobarfolder"));
    rootDir.addFile("foobar1.txt");

    FileSystemIterator iter(rootDir.getFile());
    EXPECT_TRUE(iter.isValid());
    EXPECT_STREQ("foobar1.txt", iter->getFileName().c_str());
}


TEST(FileSystemIterator, multipleFiles)
{
    TestDirectory rootDir(File("foobarfolder"));
    rootDir.addFile("foobar1.txt");
    rootDir.addFile("foobar2.txt");
    rootDir.addFile("foobar3.txt");
    rootDir.addFile("foobar4.txt");
    rootDir.addFile("foobar5.txt");

    HashSet<String> filenames;
    filenames.put("foobar1.txt");
    filenames.put("foobar2.txt");
    filenames.put("foobar3.txt");
    filenames.put("foobar4.txt");
    filenames.put("foobar5.txt");


    FileSystemIterator iter(rootDir.getFile());
    while (iter.isValid())
    {
        EXPECT_TRUE(filenames.hasElement(iter->getFileName()));
        filenames.remove(iter->getFileName());
        iter.next();
    }
    EXPECT_EQ(0u, filenames.count());
}

TEST(FileSystemIterator, emptyDir)
{
    TestDirectory rootDir(File("foobarfolder"));

    FileSystemIterator iter(rootDir.getFile());
    EXPECT_FALSE(iter.isValid());
}

TEST(FileSystemIterator, rootIsFile)
{
    TestFile rootFile(File("foobarfile"));
    FileSystemIterator iter(rootFile.getFile());

    EXPECT_FALSE(iter.isValid());
}


TEST(FileSystemIterator, rootNotExisting)
{
    FileSystemIterator iter(File("nonexistingfoobarfolder"));

    EXPECT_FALSE(iter.isValid());
}

TEST(FileSystemIterator, subSubDirs)
{
    TestDirectory rootDir(File("foobarfolder"));
    TestDirectoryPtr subDir1 = rootDir.addDirectory("subdir1");
    TestDirectoryPtr subDir2 = rootDir.addDirectory("subdir2");
    TestDirectoryPtr subDir3 = rootDir.addDirectory("subdir3");

    subDir1->addDirectory("sub1subdir1")->addFile("sub1subdir1file1")->addFile("sub1subdir1file2");
    subDir1->addDirectory("sub1subdir2")->addFile("sub1subdir2file1")->addFile("sub1subdir2file2");
    subDir1->addDirectory("sub1subdir3")->addFile("sub1subdir3file1")->addFile("sub1subdir3file2");

    subDir2->addDirectory("sub2subdir1")->addFile("sub2subdir1file1")->addFile("sub2subdir1file2");
    subDir2->addDirectory("sub2subdir2")->addFile("sub2subdir2file1")->addFile("sub2subdir2file2");
    subDir2->addDirectory("sub2subdir3")->addFile("sub2subdir3file1")->addFile("sub2subdir3file2");

    subDir3->addDirectory("sub3subdir1")->addFile("sub3subdir1file1")->addFile("sub3subdir1file2");
    subDir3->addDirectory("sub3subdir2")->addFile("sub3subdir2file1")->addFile("sub3subdir2file2");
    subDir3->addDirectory("sub3subdir3")->addFile("sub3subdir3file1")->addFile("sub3subdir3file2");


    HashSet<String> filenames;
    filenames.put("subdir1");
    filenames.put("subdir2");
    filenames.put("subdir3");
    filenames.put("sub1subdir1");
    filenames.put("sub1subdir2");
    filenames.put("sub1subdir3");
    filenames.put("sub2subdir1");
    filenames.put("sub2subdir2");
    filenames.put("sub2subdir3");
    filenames.put("sub3subdir1");
    filenames.put("sub3subdir2");
    filenames.put("sub3subdir3");
    filenames.put("sub1subdir1file1");
    filenames.put("sub1subdir2file1");
    filenames.put("sub1subdir3file1");
    filenames.put("sub2subdir1file1");
    filenames.put("sub2subdir2file1");
    filenames.put("sub2subdir3file1");
    filenames.put("sub3subdir1file1");
    filenames.put("sub3subdir2file1");
    filenames.put("sub3subdir3file1");
    filenames.put("sub1subdir1file2");
    filenames.put("sub1subdir2file2");
    filenames.put("sub1subdir3file2");
    filenames.put("sub2subdir1file2");
    filenames.put("sub2subdir2file2");
    filenames.put("sub2subdir3file2");
    filenames.put("sub3subdir1file2");
    filenames.put("sub3subdir2file2");
    filenames.put("sub3subdir3file2");

    FileSystemIterator iter(rootDir.getFile());
    while (iter.isValid())
    {
        EXPECT_TRUE(filenames.hasElement(iter->getFileName()));
        EXPECT_EQ(CAPU_OK, filenames.remove(iter->getFileName()));
        iter.next();
    }
    EXPECT_EQ(0u, filenames.count());

}
