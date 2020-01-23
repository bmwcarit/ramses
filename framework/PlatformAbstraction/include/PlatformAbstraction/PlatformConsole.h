//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMCONSOLE_H
#define RAMSES_PLATFORMCONSOLE_H

#include <cstdio>
#include <cstdarg>


namespace ramses_internal
{
    enum class ConsoleColor
    {
        Default,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
    };

    class Console
    {
    public:
        static void Print(ConsoleColor color, const char* format, ...);

        static const char* GetColor(ConsoleColor color);
        static const char* Default();
        static const char* Red();
        static const char* Green();
        static const char* Yellow();
        static const char* Blue();
        static const char* Magenta();
        static const char* Cyan();
        static const char* White();

        static void EnsureConsoleInitialized();

    private:
        static void Initialize();
    };

    inline void Console::Print(ConsoleColor color, const char* format, ...)
    {
        EnsureConsoleInitialized();
        std::va_list argptr;
        va_start(argptr, format);
        std::fputs(GetColor(color), stdout);
        std::vprintf(format, argptr);
        std::fputs(Default(), stdout);
        va_end(argptr);
    }

    inline const char* Console::GetColor(ConsoleColor color)
    {
        switch (color)
        {
        case ConsoleColor::Default: return Default();
        case ConsoleColor::Red: return Red();
        case ConsoleColor::Green: return Green();
        case ConsoleColor::Yellow: return Yellow();
        case ConsoleColor::Blue: return Blue();
        case ConsoleColor::Magenta: return Magenta();
        case ConsoleColor::Cyan: return Cyan();
        case ConsoleColor::White: return White();
        }
        return "";
    }

    inline const char* Console::Default()
    {
        return "\x1b[0m";
    }

    inline const char* Console::Red()
    {
        return "\x1b[1;31m";
    }

    inline const char* Console::Green()
    {
        return "\x1b[1;32m";
    }

    inline const char* Console::Yellow()
    {
        return "\x1b[1;33m";
    }

    inline const char* Console::Blue()
    {
        return "\x1b[1;34m";
    }

    inline const char* Console::Magenta()
    {
        return "\x1b[1;35m";
    }

    inline const char* Console::Cyan()
    {
        return "\x1b[1;36m";
    }

    inline const char* Console::White()
    {
        return "\x1b[1;37m";
    }

    inline void Console::EnsureConsoleInitialized()
    {
#ifdef _WIN32
        static auto fun = []() { Initialize(); return true; }();
        (void)fun;
#endif
    }
}

#endif
