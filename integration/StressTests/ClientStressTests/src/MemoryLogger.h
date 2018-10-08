//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYLOGGER_H
#define RAMSES_MEMORYLOGGER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/String.h"
#include "Collections/Vector.h"
#include "Utils/File.h"

class MemoryLogger
{
public:
    MemoryLogger(ramses_internal::UInt numLogsToReserve);
    void logMemory();
    void writeLogToFile(const ramses_internal::String& filePath);
    ramses_internal::UInt getTimesMemoryLogged() const;
private:
    static ramses_internal::String ReadFileContinuous(ramses_internal::File& file);
    static ramses_internal::UInt32 GetUsedMemory();
    static ramses_internal::UInt32 ReadValueFromStatusFileContent(const ramses_internal::String& statusFileContent, const ramses_internal::String& key);

    ramses_internal::Vector<ramses_internal::UInt32>    m_memUsageValues;
};

#endif
