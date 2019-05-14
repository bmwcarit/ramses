//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TABLEPRINTER_H
#define RAMSES_TABLEPRINTER_H

#include "Collections/StringOutputStream.h"
#include "Collections/Vector.h"

namespace ramses_internal {

class TablePrinter
{
public:
    TablePrinter(uint32_t tableWidth, StringOutputStream& stream)
        : m_tableWidth(tableWidth)
        , m_stream(stream)
    { };

    void addColumnDefinition(const char* name, double normalizedWidth) // Width in [0...1], expected to sum up to 1.0 for all defined columns
    {
        const uint32_t columnWidth = static_cast<uint32_t>(m_tableWidth * normalizedWidth);
        m_columnWidths.push_back(columnWidth);

        // Print the column name centered
        const int32_t delta = static_cast<int32_t>(columnWidth) - static_cast<int32_t>(strlen(name));

        addSpaces(delta / 2);
        m_stream << name;
        addSpaces(delta / 2);
    }

    template <typename T> void printValue(T value, uint32_t columnIndex);

    void nextLine()
    {
        m_stream << '\n';
    }

private:

    void addSpaces(int32_t spaceCount)
    {
        for (int32_t i = 0; i < spaceCount; i++)
        {
            m_stream << ' ';
        }
    }

    uint32_t m_tableWidth;
    StringOutputStream& m_stream;
    std::vector<uint32_t> m_columnWidths; // Width in chars
};

template <typename T>
void TablePrinter::printValue(T value, uint32_t columnIndex)
{
    const uint32_t currentLength = m_stream.length();
    m_stream << value;
    const uint32_t newLength = m_stream.length();
    const uint32_t charsWritten = newLength - currentLength;

    if (charsWritten < m_columnWidths[columnIndex])
    {
        const uint32_t delta = static_cast<int32_t>(m_columnWidths[columnIndex] - charsWritten);
        addSpaces(delta);
    }
}
}
#endif
