//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_CONSOLEINPUT_H
#define RAMSES_PLATFORMABSTRACTION_CONSOLEINPUT_H

#include <memory>

namespace ramses_internal
{
    class ConsoleInput
    {
    public:
        bool readChar(char& c);
        void interruptReadChar();

        static std::unique_ptr<ConsoleInput> TryGetUniqueConsoleInput();
        ~ConsoleInput();
    private:
        ConsoleInput();
    };
}

#endif
