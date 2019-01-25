//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MemoryLogger.h"
#include "Utils/File.h"
#include "Collections/StringOutputStream.h"
#include "Utils/FileUtils.h"

using namespace ramses_internal;

MemoryLogger::MemoryLogger(UInt numLogsToReserve)
{
    //reserve memory to avoid increasing the used memory
    m_memUsageValues.reserve(numLogsToReserve);
}

void MemoryLogger::logMemory()
{
    m_memUsageValues.push_back(GetUsedMemory());
}

void MemoryLogger::writeLogToFile(const String& filePath)
{
    File resultFile(filePath);
    StringOutputStream ss;
    for(const auto& memUsage : m_memUsageValues)
    {
        ss << memUsage << "\n";
    }
    FileUtils::CreateDirectories(resultFile);
    FileUtils::WriteAllText(resultFile, ss.release());
}

UInt MemoryLogger::getTimesMemoryLogged() const
{
    return m_memUsageValues.size();
}

String MemoryLogger::ReadFileContinuous(File& file)
{
    //read the file contents without reading the file size
    file.open(EFileMode_ReadOnly);
    UInt totalBytesRead = 0;
    Vector<char> buffer;
    if (file.isOpen())
    {
        const UInt32 readAtOnce = 1000;
        while (!file.isEof())
        {
            buffer.resize(totalBytesRead + readAtOnce);
            UInt bytesRead = 0;
            EStatus result = file.read(&(buffer[buffer.size() - readAtOnce]), readAtOnce, bytesRead);
            totalBytesRead += bytesRead;
            if (result == EStatus_RAMSES_EOF)
            {
                // read to end
                break;
            }
            if (result != EStatus_RAMSES_OK)
            {
                // an error occurred
                totalBytesRead = 0;
                break;
            }
            if (bytesRead == 0)
            {
                // read 0 bytes and no EOF (possible?)
                break;
            }
        }
        file.close();
        assert(totalBytesRead > 0 && "Could not read contents of /proc/self/status");
    }
    else
    {
        assert(false && "/proc/self/status could not be opened");
    }
    buffer.resize(totalBytesRead + 1);
    buffer[totalBytesRead] = '\0';
    return String(&buffer.front());
}

UInt32 MemoryLogger::ReadValueFromStatusFileContent(const String& fileContent, const String& key)
{
    const UInt32 fileLength = static_cast<UInt32>(fileContent.getLength());
    const UInt32 keyLength  = static_cast<UInt32>(key.getLength());
    Int keyPosition         = fileContent.find(key);
    const UInt32 valueStart = static_cast<UInt32>(keyPosition) + keyLength;
    UInt32 value            = 0;

    if (keyPosition >= 0 && fileLength > valueStart)
    {
        value = static_cast<UInt32>(atoi(fileContent.substr(valueStart, fileLength - valueStart).c_str()));
    }
    return value;
}

UInt32 MemoryLogger::GetUsedMemory()
{
    UInt32 result = 0;
    File procFile("/proc/self/status");
    //Note: to avoid this assertion (for example in Windows), use command line argument: -ms 0
    assert(procFile.exists() && "File /proc/self/status not found");
    if (procFile.exists())
    {
        //The command 'stat' cannot determine length of process status files ->
        //FileUtils::ReadAllText cannot be used
        String fileContent = ReadFileContinuous(procFile);
        result = ReadValueFromStatusFileContent(fileContent, "VmSize:");
    }
    return result;
}
