//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMAND_H
#define RAMSES_RAMSHCOMMAND_H

#include "Collections/Vector.h"
#include "Collections/String.h"
#include "Ramsh/RamshInput.h"

namespace ramses_internal
{
    class RamshCommand
    {
    public:
        virtual ~RamshCommand() {}
        void registerKeyword(const String& keyword);
        virtual Bool executeInput(const RamshInput& input) = 0;

        String keywordString() const;
        const StringVector& keywords() const;

        virtual String descriptionString() const;

    protected:
        StringVector m_keywords;
        String description;
    };

}// namespace ramses_internal

#endif
