//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INPUTSTREAMADAPTER_H
#define RAMSES_INPUTSTREAMADAPTER_H

#include "Collections/IInputStream.h"


namespace ramses_internal
{
    class InputStreamAdapter
    {
    public:
        explicit InputStreamAdapter(IInputStream& inputStream)
            : m_inputStream(inputStream)
        {
        }

        template <typename TYPE>
        void read(TYPE& inputValue) const
        {
            m_inputStream >> inputValue;
        }

        template <typename TYPE>
        TYPE read() const
        {
            TYPE value;
            read(value);
            return value;
        }


    private:
        IInputStream& m_inputStream;
    };

}

#endif
