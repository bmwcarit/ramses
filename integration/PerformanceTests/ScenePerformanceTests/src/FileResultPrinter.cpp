//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PerformanceTestBase.h"
#include "FileResultPrinter.h"
#include "JSONSerializer.h"
#include "Utils/File.h"
#include "Utils/FileUtils.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal {

FileResultPrinter::FileResultPrinter(String outputPath)
    : m_outputPath(outputPath)
{ }

void FileResultPrinter::printTestResult(const PerformanceTestBaseVector& tests, const ramses_internal::LogContext& logContext, uint32_t timePerTestSeconds)
{
    StringOutputStream stream;
    JSONSerializer serializer(stream);

    serializer.beginObject();
    {
        // Write meta data
        serializer.writeKeyValuePair("testCount", tests.size());
        serializer.writeKeyValuePair("durationPerTest", timePerTestSeconds);
        serializer.writeKeyValuePair("unixTimeInSeconds", ramses_internal::PlatformTime::GetMillisecondsAbsolute() / 1000u);

        // Array with all tests as individual objects
        serializer.beginArray("tests");
        {
            for (auto it : tests)
            {
                serializer.beginObject();
                {
                    serializer.writeKeyValuePair("name", it->getTestName());
                    serializer.writeKeyValuePair("loopCount", it->getLoopCount());
                }
                serializer.endObject();
            }
        }
        serializer.endArray();
    }
    serializer.endObject();

    File file(m_outputPath);
    if (!file.isOpen() || FileUtils::WriteAllText(file, stream.c_str()) != EStatus_RAMSES_OK)
    {
        LOG_ERROR(logContext, "Failed to write output file!");
    }
}
}
