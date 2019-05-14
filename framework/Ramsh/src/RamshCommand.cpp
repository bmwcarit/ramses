//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommand.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{

    void RamshCommand::registerKeyword(const String& keyword)
    {
        m_keywords.push_back(keyword);
    }

    ramses_internal::String RamshCommand::keywordString() const
    {
        StringOutputStream ss;
        StringVector::const_iterator iter = m_keywords.begin();
        StringVector::const_iterator end = m_keywords.end();

        while (iter != end)
        {
            ss << *iter;
            ++iter;
            if (iter != end)
            {
                ss << " | ";
            }
        }
        return ss.release();
    }

    const StringVector& RamshCommand::keywords() const
    {
        return m_keywords;
    }

    String RamshCommand::descriptionString() const
    {
        return description;
    }
}
