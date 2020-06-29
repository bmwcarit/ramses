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
#include "Utils/ArgumentBool.h"

namespace ramses_internal
{
    template<typename T>
    class Argument
    {
    public:
        Argument(const CommandLineParser& parser, const char* shortName, const char* longName, const T& default_value, const char* description = "");
        Argument(const char* shortName, const char* longName, const T& default_value, const char* description = "");

        bool wasDefined() const;
        bool hasValue() const;
        T parseValueFromCmdLine(const CommandLineParser& parser);
        String getHelpString() const;
        bool next();

        operator T() const  // NOLINT(google-explicit-constructor) implicit conversion is a (questionable) feature
        {
            return getValue();
        }

    private:
        void interpretValue(const String& valueString);
        void searchToken(const CommandLineParser& parser);
        T getValue() const;

        const char* m_shortName;
        const char* m_longName;
        const char* m_description;
        T m_defaultValue;
        std::vector<T> m_values;

        bool m_flagFound;
        bool m_valueFound;
        UInt32 m_currentValueIndex = 0u;
    };

    template<typename T>
    T ramses_internal::Argument<T>::getValue() const
    {
        if (m_values.empty())
            return m_defaultValue;

        return m_values[m_currentValueIndex];
    }

    template<typename T>
    bool ramses_internal::Argument<T>::next()
    {
        if (m_currentValueIndex + 1 < m_values.size())
        {
            ++m_currentValueIndex;
            return true;
        }

        return false;
    }

    template<typename T>
    Argument<T>::Argument(const CommandLineParser& parser, const char* shortName, const char* longName, const T& default_value, const char* description)
        : m_shortName(shortName)
        , m_longName(longName)
        , m_description(description)
        , m_defaultValue(default_value)
        , m_flagFound(false)
        , m_valueFound(false)
    {
        static_assert(!std::is_same<T, bool>::value, "use ArgumentBool");
        parseValueFromCmdLine(parser);
    }

    template<typename T>
    Argument<T>::Argument(const char* shortName, const char* longName, const T& default_value, const char* description)
        : m_shortName(shortName)
        , m_longName(longName)
        , m_description(description)
        , m_defaultValue(default_value)
        , m_flagFound(false)
        , m_valueFound(false)
    {
        static_assert(!std::is_same<T, bool>::value, "use ArgumentBool");
    }

    template<typename T>
    inline T ramses_internal::Argument<T>::parseValueFromCmdLine(const CommandLineParser& parser)
    {
        m_currentValueIndex = 0u;
        m_values.clear();

        const String shortNameDashed(String("-") + m_shortName);
        const String longNameDashed(String("--") + m_longName);

        UInt32 searchIndex = 0u;
        const CommandLineArgument* cmdLineArg = parser.getOption(shortNameDashed, longNameDashed, true, &searchIndex);
        while (nullptr != cmdLineArg)
        {
            m_flagFound = true;

            if (cmdLineArg->hasValue())
            {
                m_valueFound = true;
                interpretValue(cmdLineArg->getValue());
            }

            cmdLineArg = parser.getOption(shortNameDashed, longNameDashed, true, &searchIndex);
        }

        return getValue();
    }

    template<typename T>
    inline String ramses_internal::Argument<T>::getHelpString() const
    {
        StringOutputStream stream;
        stream << "-" << m_shortName << ", --" << m_longName;
        stream << " <value>";
        stream << " \t" << m_description << " (default: " << m_defaultValue << ")\n";
        return stream.c_str();
    }

    template<typename T>
    inline bool Argument<T>::wasDefined() const
    {
        return m_flagFound;
    }

    template<typename T>
    inline bool Argument<T>::hasValue() const
    {
        return m_valueFound;
    }

    typedef Argument<Float>     ArgumentFloat;
    typedef Argument<Int32>     ArgumentInt32;
    typedef Argument<String>    ArgumentString;
    typedef Argument<UInt16>    ArgumentUInt16;
    typedef Argument<UInt32>    ArgumentUInt32;
    typedef Argument<Vector3>   ArgumentVec3;

    template<>
    inline void Argument<Float>::interpretValue(const String& valueString)
    {
        m_values.push_back(static_cast<Float>(atof(valueString.c_str())));
    }

    template<>
    inline void Argument<String>::interpretValue(const String& valueString)
    {
        m_values.push_back(valueString);
    }

    template<>
    inline void Argument<Int32>::interpretValue(const String& valueString)
    {
        m_values.push_back(atoi(valueString.c_str()));
    }

    template<>
    inline void Argument<UInt16>::interpretValue(const String& valueString)
    {
        const int value = atoi(valueString.c_str());
        if (value >= 0 && static_cast<unsigned int>(value) <= std::numeric_limits<uint16_t>::max())
        {
            m_values.push_back(static_cast<uint16_t>(value));
        }
    }

    template<>
    inline void Argument<UInt32>::interpretValue(const String& valueString)
    {
        const long long value = atoll(valueString.c_str());
        if (value >= 0 && static_cast<unsigned long long>(value) <= std::numeric_limits<uint32_t>::max())
        {
            m_values.push_back(static_cast<uint32_t>(value));
        }
    }

    template<>
    inline void Argument<Vector3>::interpretValue(const String& valueString)
    {
        if (valueString.startsWith("[") && valueString.endsWith("]"))
        {
            auto withoutBrackets = valueString.substr(1, valueString.size() - 2);

            size_t componentsFound = 0;
            float components[3];
            auto putComponentsToVector = [&](const char* vectorComponentAsString) {
                if(componentsFound < 3)
                    components[componentsFound] = static_cast<Float>(atof(vectorComponentAsString));
                ++componentsFound;
            };

            InplaceStringTokenizer::TokenizeToCStrings(withoutBrackets, withoutBrackets.size(), ',', putComponentsToVector);

            if (3 == componentsFound)
                m_values.emplace_back(components[0], components[1], components[2]);
        }
    }
}

#endif
