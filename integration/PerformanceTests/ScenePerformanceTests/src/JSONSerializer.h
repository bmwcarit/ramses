//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_JSONSERIALIZER_H
#define RAMSES_JSONSERIALIZER_H

#include "Collections/StringOutputStream.h"
#include "Collections/Vector.h"

namespace ramses_internal {

class JSONSerializer
{
public:

    JSONSerializer(StringOutputStream& stream);

    void beginObject();
    void endObject();

    void beginArray(const char* arrayName);
    void endArray();

    template <typename T> void writeKeyValuePair(const char* key, const T& value);

private:
    void beginLine();
    void writeIndentation();
    void closeScope(char value);

    StringOutputStream& m_stream;
    std::vector<bool> m_addDelimiter;
};

template <typename T>
void JSONSerializer::writeKeyValuePair(const char* key, const T& value)
{
    beginLine();

    m_stream << '"';
    m_stream << key;
    m_stream << '"';
    m_stream << ": ";

    m_stream << '"';
    m_stream << value;
    m_stream << '"';
}
}
#endif
