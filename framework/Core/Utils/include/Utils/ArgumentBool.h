//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARGUMENTBOOL_H
#define RAMSES_ARGUMENTBOOL_H

#include "Collections/String.h"
#include <PlatformAbstraction/PlatformTypes.h>
#include "Utils/CommandLineParser.h"
#include "Collections/StringOutputStream.h"
#include "PlatformAbstraction/Macros.h"

namespace ramses_internal
{
    class ArgumentBool
    {
    public:
        ArgumentBool(const CommandLineParser& parser, const char* shortName, const char* longName, const char* description = "");
        ArgumentBool(const char* shortName, const char* longName, const char* description = "");

        RNODISCARD bool wasDefined() const;
        RNODISCARD std::string getHelpString() const;
        bool parseFromCmdLine(const CommandLineParser& parser);

        operator bool() const;  // NOLINT(google-explicit-constructor) implicit conversion is a (questionable) feature

    private:
        const char* m_shortName;
        const char* m_longName;
        const char* m_description;

        bool m_flagFound;
    };

    inline ArgumentBool::ArgumentBool(const CommandLineParser& parser, const char* shortName, const char* longName, const char* description)
        : m_shortName(shortName)
        , m_longName(longName)
        , m_description(description)
        , m_flagFound(false)
    {
        parseFromCmdLine(parser);
    }

    inline ArgumentBool::ArgumentBool(const char* shortName, const char* longName, const char* description)
        : m_shortName(shortName)
        , m_longName(longName)
        , m_description(description)
        , m_flagFound(false)
    {
    }

    inline std::string ArgumentBool::getHelpString() const
    {
        StringOutputStream stream;
        stream << "-" << m_shortName << ", --" << m_longName;
        stream << " \t" << m_description << " (default: false)\n";
        return stream.release();
    }

    inline bool ArgumentBool::wasDefined() const
    {
        return m_flagFound;
    }

    inline ArgumentBool::operator bool() const
    {
        return wasDefined();
    }

    inline bool ArgumentBool::parseFromCmdLine(const CommandLineParser& parser)
    {
        const String shortNameDashed(String("-") + m_shortName);
        const String longNameDashed(String("--") + m_longName);
        const CommandLineArgument* cmdLineArg = parser.getOption(shortNameDashed, longNameDashed, false);
        m_flagFound = (cmdLineArg != nullptr);
        return m_flagFound;
    }
}

#endif
