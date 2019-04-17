//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <Utils/CommandLineParser.h>
#include "Utils/StringUtils.h"

namespace ramses_internal
{
    CommandLineParser::CommandLineParser(Int argc, char const* const* argv)
    {
        if (argc > 0 && 0 != argv)
        {
            m_programName = argv[0];
            for (Int index = 1; index < argc; ++index)
            {
                const Bool startsWithDash = (argv[index] && argv[index][0] == '-');

                CommandLineArgument cmdLineArg;
                cmdLineArg.setName(startsWithDash ? StringUtils::Trim(argv[index]) : argv[index]);

                if (index + 1 < argc && startsWithDash)
                {
                    const String nextElement(argv[index + 1]);

                    char* endptr = 0;
                    const Double unusedValue = strtod(nextElement.c_str(), &endptr);
                    UNUSED(unusedValue);

                    const Bool isNumber = endptr && (*endptr == 0);
                    const Bool doesNotStartWithDash = (nextElement.at(0) != '-');

                    if (doesNotStartWithDash || isNumber)
                    {
                        cmdLineArg.setValue(nextElement);
                        index++;
                    }
                }
                m_args.push_back(cmdLineArg);
            }
        }
    }

    StringVector CommandLineParser::getNonOptions() const
    {
        StringVector argNames;
        for (auto& arg : m_args)
        {
            if (!arg.hasBeenUsed())
            {
                argNames.push_back(arg.getName());
            }
        }
        return argNames;
    }

    const String& CommandLineParser::getProgramName() const
    {
        return m_programName;
    }

    const CommandLineArgument* CommandLineParser::getOption(const String& shortname, const String& longname, Bool canHaveValue) const
    {
        CommandLineArgument* argument = NULL;
        for (auto& current : m_args)
        {
            if (current.getName() == shortname || current.getName() == longname)
            {
                current.setUsed();
                argument = &current;
                break;
            }
        }

        if (argument != nullptr && argument->hasValue() && !canHaveValue)
        {
            // wrongly found as argument, must be file argument instead
            CommandLineArgument newArgument;
            newArgument.setName(argument->getValue());
            argument->setValue("");
            m_args.push_back(newArgument);
        }

        for (auto& current : m_args)
        {
            if (current.getName() == shortname || current.getName() == longname)
            {
                return &current;
            }
        }

        return nullptr;
    }
}
