//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshInput.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    RamshInput::RamshInput()
    {
    }

    void RamshInput::append(const Char c)
    {
        Char tmp[] = { c, '\0' };
        this->push_back(tmp);
    }

    void RamshInput::append(const String& c)
    {
        this->push_back(c);
    }
    void RamshInput::append(const Char* c)
    {
        this->push_back(c);
    }

    ramses_internal::Bool RamshInput::isValid() const
    {
        return (this->begin() != this->end());
    }

    ramses_internal::String RamshInput::toString() const
    {
        StringOutputStream cmdStream;
        {
            StringVector::const_iterator iter = this->begin();
            StringVector::const_iterator end = this->end();
            for (; iter != end; ++iter)
            {
                cmdStream << *iter << " ";
            }
        }
        return cmdStream.release();
    }

    const ramses_internal::String& RamshInput::operator[](const UInt index) const
    {
        return StringVector::operator[](index);
    }

    ramses_internal::String& RamshInput::operator[](const UInt index)
    {
        return StringVector::operator[](index);
    }

    ramses_internal::Bool RamshInput::operator==(const RamshInput& other) const
    {
        return static_cast<const StringVector&>(*this) == other;
    }

}
