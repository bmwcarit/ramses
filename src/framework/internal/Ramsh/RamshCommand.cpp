//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Ramsh/RamshCommand.h"
#include "fmt/format.h"

namespace ramses::internal
{

    void RamshCommand::registerKeyword(const std::string& keyword)
    {
        m_keywords.push_back(keyword);
    }

    std::string RamshCommand::keywordString() const
    {
        return fmt::format("{}", fmt::join(m_keywords, " | "));
    }

    const std::vector<std::string>& RamshCommand::keywords() const
    {
        return m_keywords;
    }

    std::string RamshCommand::descriptionString() const
    {
        return description;
    }
}
