//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "JSONSerializer.h"

namespace ramses_internal {

JSONSerializer::JSONSerializer(StringOutputStream& stream)
    : m_stream(stream)
{
}

void JSONSerializer::beginObject()
{
    beginLine();
    m_stream << "{";
    m_addDelimiter.push_back(false);
}

void JSONSerializer::endObject()
{
    closeScope('}');
}

void JSONSerializer::beginArray(const char* arrayName)
{
    beginLine();
    m_stream << '"';
    m_stream << arrayName;
    m_stream << '"';
    m_stream << ": [";
    m_addDelimiter.push_back(false);
}

void JSONSerializer::endArray()
{
    closeScope(']');
}

void JSONSerializer::closeScope(char value)
{
    m_stream << '\n';
    m_addDelimiter.pop_back();
    writeIndentation();
    m_stream << value;
}

void JSONSerializer::beginLine()
{
    // Special case of avoiding initial linebreak
    if (m_addDelimiter.empty())
    {
        return;
    }

    if (m_addDelimiter.size() > 0 && m_addDelimiter[m_addDelimiter.size() - 1])
    {
        m_stream << ',';
    }

    m_stream << '\n';

    writeIndentation();

    // Make sure next item in the same scope will include the delimiter
    if (m_addDelimiter.size() > 0)
    {
        m_addDelimiter[m_addDelimiter.size() - 1] = true;
    }
}

void JSONSerializer::writeIndentation()
{
    const uint32_t IndentSize = 4;

    for (uint32_t i = 0; i < m_addDelimiter.size() * IndentSize; i++)
    {
        m_stream << ' ';
    }
}
}
