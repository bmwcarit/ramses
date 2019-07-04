//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARGUMENT_H
#define RAMSES_ARGUMENT_H

#include "Collections/String.h"
#include "Collections/Vector.h"
#include <PlatformAbstraction/PlatformTypes.h>
#include "Utils/CommandLineParser.h"
#include "Collections/StringOutputStream.h"
#include "Math3d/Vector3.h"
#include "InplaceStringTokenizer.h"

namespace ramses_internal
{
    template<typename T>
    class Argument
    {
    public:
        Argument<T>(const CommandLineParser& parser, const char* shortName, const char* longName, const T& default_value, const char* description = "");
        Argument<T>(const char* shortName, const char* longName, const T& default_value, const char* description = "");

        Bool wasDefined() const;
        Bool hasValue() const;
        T parseValueFromCmdLine(const CommandLineParser& parser);
        String getHelpString() const;

        operator T() const
        {
            return m_value;
        }

    private:
        const String& getUnconvertedString() const;
        void interpretValue();
        void searchToken(const CommandLineParser& parser);
        Bool canHaveArgument() const;

        const char* m_shortName;
        const char* m_longName;
        const char* m_description;
        T m_defaultValue;
        T m_value;

        String m_unconvertedValueString;
        Bool m_flagFound;
        Bool m_valueFound;
    };

    template<typename T>
    inline Argument<T>::Argument(const CommandLineParser& parser, const char* shortName, const char* longName, const T& default_value, const char* description)
        : m_shortName(shortName)
        , m_longName(longName)
        , m_description(description)
        , m_defaultValue(default_value)
        , m_value(default_value)
        , m_flagFound(false)
        , m_valueFound(false)
    {
        parseValueFromCmdLine(parser);
    }

    template<typename T>
    inline Argument<T>::Argument(const char* shortName, const char* longName, const T& default_value, const char* description)
        : m_shortName(shortName)
        , m_longName(longName)
        , m_description(description)
        , m_defaultValue(default_value)
        , m_value(default_value)
        , m_flagFound(false)
        , m_valueFound(false)
    {
    }

    template<typename T>
    inline T ramses_internal::Argument<T>::parseValueFromCmdLine(const CommandLineParser& parser)
    {
        searchToken(parser);
        interpretValue();
        return m_value;
    }

    template<typename T>
    inline String ramses_internal::Argument<T>::getHelpString() const
    {
        StringOutputStream stream;
        stream << "-" << m_shortName << ", --" << m_longName;
        if (canHaveArgument())
        {
            stream << " <value>";
        }
        stream << " \t" << m_description << " (default: " << m_defaultValue << ")\n";
        return stream.c_str();
    }

    template<typename T>
    inline void Argument<T>::searchToken(const CommandLineParser& parser)
    {
        const String shortNameDashed(String("-") + m_shortName);
        const String longNameDashed(String("--") + m_longName);

        const CommandLineArgument* cmdLineArg = parser.getOption(shortNameDashed, longNameDashed, canHaveArgument());
        if (0 != cmdLineArg)
        {
            m_flagFound = true;

            if (cmdLineArg->hasValue())
            {
                m_unconvertedValueString = cmdLineArg->getValue();
                m_valueFound = true;
            }
        }
    }

    template<typename T>
    inline Bool Argument<T>::wasDefined() const
    {
        return m_flagFound;
    }

    template<typename T>
    inline Bool Argument<T>::hasValue() const
    {
        return m_valueFound;
    }

    template<typename T>
    inline const String& Argument<T>::getUnconvertedString() const
    {
        return m_unconvertedValueString;
    }

    template<typename T>
    inline Bool Argument<T>::canHaveArgument() const
    {
        return true;
    }

    template<>
    inline Bool Argument<Bool>::canHaveArgument() const
    {
        return false;
    }

    typedef Argument<Bool>      ArgumentBool;
    typedef Argument<Float>     ArgumentFloat;
    typedef Argument<Int32>     ArgumentInt32;
    typedef Argument<String>    ArgumentString;
    typedef Argument<UInt16>    ArgumentUInt16;
    typedef Argument<UInt32>    ArgumentUInt32;
    typedef Argument<Vector3>    ArgumentVec3;

    template<>
    inline void Argument<Float>::interpretValue()
    {
        if (hasValue())
        {
            m_value = static_cast<Float>(atof(getUnconvertedString().c_str()));
        }
    }

    template<>
    inline void Argument<String>::interpretValue()
    {
        if (hasValue())
        {
            m_value = getUnconvertedString();
        }
    }

    template<>
    inline void Argument<Bool>::interpretValue()
    {
        if (wasDefined())
        {
            m_value = !m_defaultValue;
        }
    }

    template<>
    inline void Argument<Int32>::interpretValue()
    {
        if (hasValue())
        {
            m_value = atoi(getUnconvertedString().c_str());
        }
    }

    template<>
    inline void Argument<UInt16>::interpretValue()
    {
        if (hasValue())
        {
            const int value = atoi(getUnconvertedString().c_str());
            if (value >= 0 && static_cast<unsigned int>(value) <= std::numeric_limits<uint16_t>::max())
            {
                m_value = static_cast<uint16_t>(value);
            }
        }
    }

    template<>
    inline void Argument<UInt32>::interpretValue()
    {
        if (hasValue())
        {
            const long long value = atoll(getUnconvertedString().c_str());
            if (value >= 0 && static_cast<unsigned long long>(value) <= std::numeric_limits<uint32_t>::max())
            {
                m_value = static_cast<uint32_t>(value);
            }
        }
    }

    template<>
    inline void Argument<Vector3>::interpretValue()
    {
        if (hasValue())
        {
            const auto& string = getUnconvertedString();

            if (string.startsWith("[") && string.endsWith("]"))
            {
                auto withoutBrackets = string.substr(1, string.getLength() - 2);

                size_t componentsFound = 0;
                float components[3];
                auto putComponentsToVector = [&](const char* vectorComponentAsString) {
                    if(componentsFound < 3)
                        components[componentsFound] = static_cast<Float>(atof(vectorComponentAsString));
                    ++componentsFound;
                };

                InplaceStringTokenizer::TokenizeToCStrings(withoutBrackets, withoutBrackets.getLength(), ',', putComponentsToVector);

                if (3 == componentsFound)
                    memcpy(m_value.data, components, 3 * sizeof(float));
            }
        }
    }
}

#endif
