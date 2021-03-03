//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMAND_H
#define RAMSES_RAMSHCOMMAND_H

#include <string>
#include <vector>

namespace ramses_internal
{
    class RamshCommand
    {
    public:
        virtual ~RamshCommand() = default;
        void registerKeyword(const std::string& keyword);
        virtual bool executeInput(const std::vector<std::string>& input) = 0;

        std::string keywordString() const;
        const std::vector<std::string>& keywords() const;

        virtual std::string descriptionString() const;

    protected:
        std::vector<std::string> m_keywords;
        std::string description;
    };
}// namespace ramses_internal

#endif
