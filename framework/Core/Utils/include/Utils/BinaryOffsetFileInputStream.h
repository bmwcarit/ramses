//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_BINARYOFFSETFILEINPUTSTREAM_H
#define RAMSES_FRAMEWORK_BINARYOFFSETFILEINPUTSTREAM_H

#include "Collections/IInputStream.h"
#include <cstdio>

namespace ramses_internal
{
    /*
     * Special input stream that works on an already open filedescriptor. The relevant
     * content inside the referenced file may not start at beginning of the file but may
     * start at an arbitrary offset. Also the file may be longer then the relevant content.
     *
     * The class takes ownership if the passed filedescriptor.
     */
    class BinaryOffsetFileInputStream : public IInputStream
    {
    public:
        explicit BinaryOffsetFileInputStream(int fd, size_t offset, size_t length);
        ~BinaryOffsetFileInputStream() override;

        BinaryOffsetFileInputStream(const BinaryOffsetFileInputStream&) = delete;
        BinaryOffsetFileInputStream& operator=(const BinaryOffsetFileInputStream&) = delete;

        IInputStream& read(void* buffer, size_t size) override;

        EStatus seek(Int numberOfBytesToSeek, Seek origin) override;
        EStatus getPos(size_t& position) const override;

        [[nodiscard]] EStatus getState() const override;

    private:
        FILE* m_file;
        EStatus m_state;
        const size_t m_startPos;
        const size_t m_length;
        size_t m_pos;
    };
}

#endif
