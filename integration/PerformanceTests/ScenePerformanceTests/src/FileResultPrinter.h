//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FILERESULTPRINTER_H
#define RAMSES_FILERESULTPRINTER_H

#include "Collections/StringOutputStream.h"
#include "Utils/LogContext.h"

namespace ramses_internal {

class FileResultPrinter
{
public:

    FileResultPrinter(String outputPath);
    void printTestResult(const PerformanceTestBaseVector& tests, const ramses_internal::LogContext& logContext, uint32_t timePerTestSeconds);

private:
    String m_outputPath;
};
}
#endif
