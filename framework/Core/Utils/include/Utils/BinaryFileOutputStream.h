//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYFILEOUTPUTSTREAM_H
#define RAMSES_BINARYFILEOUTPUTSTREAM_H

#include "Collections/IOutputStream.h"
#include "Utils/File.h"

namespace ramses_internal
{
    class BinaryFileOutputStream: public IOutputStream
    {
    public:
        explicit BinaryFileOutputStream(File& file, File::Mode mode = File::Mode::WriteNewBinary);
        ~BinaryFileOutputStream() override;

        BinaryFileOutputStream(const BinaryFileOutputStream&) = delete;
        BinaryFileOutputStream& operator=(const BinaryFileOutputStream&) = delete;

        IOutputStream& write(const void* data, size_t size) override;

        EStatus getPos(size_t& position) const override;
        EStatus getState() const;

    private:
        File& m_file;
        EStatus m_state;
    };

    inline
    BinaryFileOutputStream::BinaryFileOutputStream(File& file, File::Mode mode)
        : m_file(file)
        , m_state(m_file.open(mode) ? EStatus::Ok : EStatus::Error)
    {
    }

    inline
    BinaryFileOutputStream::~BinaryFileOutputStream()
    {
        m_file.flush();
        m_file.close();
    }

    inline
    IOutputStream&
    BinaryFileOutputStream::write(const void* data, size_t size)
    {
        if (EStatus::Ok == m_state)
        {
            m_state = m_file.write(data, size) ? EStatus::Ok : EStatus::Error;
        }
        return *this;
    }

    inline
    ramses_internal::EStatus BinaryFileOutputStream::getPos(size_t& position) const
    {
        const_cast<EStatus&>(m_state) = m_file.getPos(position) ? EStatus::Ok : EStatus::Error;
        return m_state;
    }

    inline
    EStatus BinaryFileOutputStream::getState() const
    {
        return m_state;
    }
}

#endif
