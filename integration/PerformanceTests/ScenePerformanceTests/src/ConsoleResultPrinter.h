//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONSOLERESULTPRINTER_H
#define RAMSES_CONSOLERESULTPRINTER_H

#include "Utils/LogContext.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal {

class ConsoleResultPrinter
{
public:

    ConsoleResultPrinter(const ramses_internal::LogContext& logContext);
    void printTestResult(const PerformanceTestBaseVector& tests);

private:

    static const char* GetAssertDisplayString(const PerformanceTestBase& test);
    static String GetMemoryDisplayString(const PerformanceTestBase& test);
    static void AppendByteSizeFormat(uint64_t byteCount, StringOutputStream& stream);

    const ramses_internal::LogContext& m_logContext;
};
}
#endif
